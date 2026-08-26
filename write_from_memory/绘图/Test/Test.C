/*
 * 功能：读取微分截面与接受度 CSV 数据，绘制共享横轴且上下对齐的两幅子图，并输出 Test.pdf。
 * 方法：上图用 TGraphErrors 绘制微分截面及 Y 误差，下图用 TGraph 绘制 Acceptance；统一 pad 边距和 X 轴，并按 pad 边距动态放置紧凑图例与面板编号。
 * 注意事项：CrossSection1/error1 对应基态，CrossSection2/error2 对应第一激发态；输入使用固定绝对路径，运行时会覆盖原有 Test.pdf。
 */

#include "TCanvas.h"
#include "TError.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TPad.h"
#include "TStyle.h"
#include "TString.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char *kCrossSectionFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/LabCrossSection.csv";
const char *kGroundStateAcceptanceFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/SolidAngle_gs.csv";
const char *kExcitedStateAcceptanceFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/SolidAngle_ex.csv";
const char *kOutputFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/Test.pdf";
constexpr Double_t kPanelInset = 0.02;
constexpr Double_t kLegendWidth = 0.30;
constexpr Double_t kLegendEntryHeight = 0.055;
constexpr Double_t kLegendTextSize = 0.047;
constexpr Int_t kLegendEntryCount = 2;

// 读取微分截面 CSV，并分别保存基态和第一激发态的数值及 Y 误差。
Bool_t ReadCrossSections(std::vector<Double_t> &angles,
                         std::vector<Double_t> &groundValues,
                         std::vector<Double_t> &groundErrors,
                         std::vector<Double_t> &excitedValues,
                         std::vector<Double_t> &excitedErrors)
{
    std::ifstream input(kCrossSectionFile);
    if (!input.is_open()) {
        Error("ReadCrossSections", "无法打开 %s", kCrossSectionFile);
        return kFALSE;
    }

    std::string line;
    std::getline(input, line);
    while (std::getline(input, line)) {
        std::replace(line.begin(), line.end(), ',', ' ');

        Double_t angle = 0.0;
        Double_t groundValue = 0.0;
        Double_t groundError = 0.0;
        Double_t excitedValue = 0.0;
        Double_t excitedError = 0.0;
        std::string angleBin;
        std::istringstream parser(line);

        if (!(parser >> angle >> angleBin >> groundValue >> groundError
                     >> excitedValue >> excitedError))
            continue;

        angles.push_back(angle);
        groundValues.push_back(groundValue);
        groundErrors.push_back(groundError);
        excitedValues.push_back(excitedValue);
        excitedErrors.push_back(excitedError);
    }

    if (angles.empty())
        Error("ReadCrossSections", "文件中没有可用数据：%s", kCrossSectionFile);
    return !angles.empty();
}

// 读取一个接受度 CSV，并将 Angle Bin 转换为区间中心角度。
Bool_t ReadAcceptance(const char *fileName, std::vector<Double_t> &angles,
                      std::vector<Double_t> &acceptance)
{
    std::ifstream input(fileName);
    if (!input.is_open()) {
        Error("ReadAcceptance", "无法打开 %s", fileName);
        return kFALSE;
    }

    std::string line;
    std::getline(input, line);
    while (std::getline(input, line)) {
        std::replace(line.begin(), line.end(), ',', ' ');

        std::string angleBin;
        Double_t detected = 0.0;
        Double_t generated = 0.0;
        Double_t solidAngle = 0.0;
        Double_t geometricSolidAngle = 0.0;
        Double_t value = 0.0;
        std::istringstream parser(line);

        if (!(parser >> angleBin >> detected >> generated >> solidAngle
                     >> geometricSolidAngle >> value))
            continue;

        const std::size_t separator = angleBin.find('_');
        if (separator == std::string::npos)
            continue;

        const Double_t lowerEdge = std::stod(angleBin.substr(0, separator));
        const Double_t upperEdge = std::stod(angleBin.substr(separator + 1));
        angles.push_back(0.5 * (lowerEdge + upperEdge));
        acceptance.push_back(value);
    }

    if (angles.empty())
        Error("ReadAcceptance", "文件中没有可用数据：%s", fileName);
    return !angles.empty();
}

// 配置单个子图的边距、边框、刻度以及 Y 轴线性或对数模式。
void ConfigurePad(TPad *pad, Double_t topMargin, Double_t bottomMargin,
                  Bool_t logY)
{
    pad->SetFillColor(kWhite);
    pad->SetFillStyle(1001);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);
    pad->SetLeftMargin(0.16);
    pad->SetRightMargin(0.035);
    pad->SetTopMargin(topMargin);
    pad->SetBottomMargin(bottomMargin);
    pad->SetLogy(logY);
    pad->SetTickx(1);
    pad->SetTicky(1);
}

// 配置坐标框的公共样式，并根据参数决定是否显示 X 轴标签和标题。
void ConfigureFrame(TH1D *frame, Bool_t showXAxis)
{
    frame->SetLineWidth(2);
    frame->GetXaxis()->SetRangeUser(46.0, 74.0);
    frame->GetXaxis()->SetNdivisions(506);
    frame->GetXaxis()->SetTickLength(0.030);

    if (showXAxis) {
        frame->GetXaxis()->SetLabelFont(42);
        frame->GetXaxis()->SetLabelSize(0.058);
        frame->GetXaxis()->SetLabelOffset(0.012);
        frame->GetXaxis()->SetTitle("#theta_{lab} (deg)");
        frame->GetXaxis()->SetTitleFont(42);
        frame->GetXaxis()->SetTitleSize(0.065);
        frame->GetXaxis()->SetTitleOffset(1.00);
    } else {
        frame->GetXaxis()->SetLabelSize(0.0);
        frame->GetXaxis()->SetTitleSize(0.0);
    }

    frame->GetYaxis()->SetLabelFont(42);
    frame->GetYaxis()->SetLabelSize(0.060);
    frame->GetYaxis()->SetTitle("");
    frame->GetYaxis()->SetTickLength(0.018);
    frame->GetYaxis()->SetLabelOffset(0.008);
}

// 设置数据图的标记形状、颜色、大小及误差棒线宽。
void ConfigureGraph(TGraph *graph, Int_t markerStyle, Color_t color)
{
    graph->SetMarkerStyle(markerStyle);
    graph->SetMarkerSize(0.8);
    graph->SetMarkerColor(color);
    graph->SetLineColor(color);
    graph->SetLineWidth(1);
}

// 根据 pad 边距在左上角绘制紧凑图例，并在右上角绘制顺序面板编号。
void DrawAnnotations(TPad *pad, TGraph *groundGraph, TGraph *excitedGraph,
                     Int_t panelIndex)
{
    const Double_t legendLeft = pad->GetLeftMargin() + kPanelInset;
    const Double_t annotationTop =
        1.0 - pad->GetTopMargin() - kPanelInset;
    TLegend *legend = new TLegend(
        legendLeft,
        annotationTop - kLegendEntryHeight * kLegendEntryCount,
        legendLeft + kLegendWidth, annotationTop);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextFont(42);
    legend->SetTextSize(kLegendTextSize);
    legend->SetEntrySeparation(0.0);
    legend->AddEntry(groundGraph, "g.s.   0^{+}", "p");
    legend->AddEntry(excitedGraph, "3.368 MeV   2^{+}", "p");
    legend->Draw();

    TLatex panel;
    panel.SetNDC();
    panel.SetTextFont(42);
    panel.SetTextSize(0.070);
    panel.SetTextAlign(33);
    panel.DrawLatex(1.0 - pad->GetRightMargin() - kPanelInset,
                    annotationTop, Form("(%c)", 'a' + panelIndex));
}

// 在画布左侧分别绘制微分截面和接受度的 Y 轴标题。
void DrawYAxisTitles()
{
    TLatex title;
    title.SetNDC();
    title.SetTextFont(42);
    title.SetTextSize(0.045);
    title.SetTextAlign(22);
    title.SetTextAngle(90);
    title.DrawLatex(0.047, 0.75, "d#sigma/d#Omega (mb/sr)");
    title.DrawLatex(0.047, 0.25, "Acceptance");
}

} // namespace

// 读取三个 CSV，绘制上下两幅实验数据图，并将结果保存为 Test.pdf。
void Test()
{
    std::vector<Double_t> crossSectionAngles;
    std::vector<Double_t> groundCrossSections;
    std::vector<Double_t> groundCrossSectionErrors;
    std::vector<Double_t> excitedCrossSections;
    std::vector<Double_t> excitedCrossSectionErrors;
    std::vector<Double_t> groundAcceptanceAngles;
    std::vector<Double_t> groundAcceptance;
    std::vector<Double_t> excitedAcceptanceAngles;
    std::vector<Double_t> excitedAcceptance;

    if (!ReadCrossSections(crossSectionAngles, groundCrossSections,
                           groundCrossSectionErrors, excitedCrossSections,
                           excitedCrossSectionErrors))
        return;
    if (!ReadAcceptance(kGroundStateAcceptanceFile, groundAcceptanceAngles,
                        groundAcceptance))
        return;
    if (!ReadAcceptance(kExcitedStateAcceptanceFile, excitedAcceptanceAngles,
                        excitedAcceptance))
        return;

    const std::vector<Double_t> zeroErrors(crossSectionAngles.size(), 0.0);
    TGraphErrors *groundCrossSectionGraph = new TGraphErrors(
        crossSectionAngles.size(), crossSectionAngles.data(),
        groundCrossSections.data(), zeroErrors.data(),
        groundCrossSectionErrors.data());
    TGraphErrors *excitedCrossSectionGraph = new TGraphErrors(
        crossSectionAngles.size(), crossSectionAngles.data(),
        excitedCrossSections.data(), zeroErrors.data(),
        excitedCrossSectionErrors.data());
    TGraph *groundAcceptanceGraph = new TGraph(
        groundAcceptanceAngles.size(), groundAcceptanceAngles.data(),
        groundAcceptance.data());
    TGraph *excitedAcceptanceGraph = new TGraph(
        excitedAcceptanceAngles.size(), excitedAcceptanceAngles.data(),
        excitedAcceptance.data());

    ConfigureGraph(groundCrossSectionGraph, 24, kBlack);
    ConfigureGraph(excitedCrossSectionGraph, 25, kRed + 1);
    ConfigureGraph(groundAcceptanceGraph, 24, kBlack);
    ConfigureGraph(excitedAcceptanceGraph, 25, kRed + 1);

    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    TGaxis::SetMaxDigits(3);

    TCanvas *c1 = new TCanvas("c1", "c1", 650, 650);
    c1->SetFillColor(kWhite);
    c1->SetBorderMode(0);

    TPad *upperPad = new TPad("upperPadTest", "", 0.0, 0.500, 1.0, 1.000);
    TPad *lowerPad = new TPad("lowerPadTest", "", 0.0, 0.000, 1.0, 0.502);
    ConfigurePad(upperPad, 0.035, 0.000, kTRUE);
    ConfigurePad(lowerPad, 0.000, 0.145, kFALSE);
    upperPad->Draw();
    lowerPad->Draw();

    TH1D *h1 = new TH1D("h1", "", 410, 40.0, 81.0);
    ConfigureFrame(h1, kFALSE);
    h1->SetMinimum(5e-4);
    h1->SetMaximum(10.0);
    h1->GetYaxis()->SetNdivisions(505);
    upperPad->cd();
    h1->Draw("AXIS");
    groundCrossSectionGraph->Draw("P E1 SAME");
    excitedCrossSectionGraph->Draw("P E1 SAME");
    DrawAnnotations(upperPad, groundCrossSectionGraph,
                    excitedCrossSectionGraph, 0);
    upperPad->RedrawAxis();

    TH1D *h2 = new TH1D("h2", "", 410, 40.0, 81.0);
    ConfigureFrame(h2, kTRUE);
    h2->SetMinimum(0.0);
    h2->SetMaximum(0.12);
    h2->GetYaxis()->SetNdivisions(506, kFALSE);
    h2->GetYaxis()->ChangeLabel(-1, -1, 0.0);
    lowerPad->cd();
    h2->Draw("AXIS");
    groundAcceptanceGraph->Draw("P SAME");
    excitedAcceptanceGraph->Draw("P SAME");
    DrawAnnotations(lowerPad, groundAcceptanceGraph,
                    excitedAcceptanceGraph, 1);
    lowerPad->RedrawAxis();

    c1->cd();
    DrawYAxisTitles();
    c1->Modified();
    c1->Update();
    c1->SaveAs(kOutputFile);
}
