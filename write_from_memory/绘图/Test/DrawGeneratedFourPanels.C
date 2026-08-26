/*
 * 功能：读取 GeneratedEx.root 中 tree 的 Ex1 至 Ex4 vector<double> 分支，绘制两列两行共享横轴图并输出 GeneratedFourPanels.pdf。
 * 方法：左列显示 Ex1/Ex2 的 -10 至 10 MeV 分布，右列显示 Ex3/Ex4 的 10 至 30 MeV 分布；四幅 TH1D 使用独立 Y 范围和完整统计框。
 * 注意事项：每列仅在下图显示 X 轴标签和标题；Y 标题由实际 bin width 生成，高斯尾部落入横轴范围外时只计入 underflow/overflow。
 */

#include "TCanvas.h"
#include "TError.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TPad.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"

#include <vector>

namespace {

const char *kInputFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/GeneratedEx.root";
const char *kOutputFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/GeneratedFourPanels.pdf";
constexpr Int_t kPanelCount = 4;
constexpr Int_t kColumnCount = 2;
constexpr Int_t kCanvasWidth = 1300;
constexpr Int_t kCanvasHeight = 800;
constexpr Double_t kPaperWidthCm = 26.0;
constexpr Double_t kPaperHeightCm = 16.0;
constexpr Double_t kColumnGap = 0.01;
constexpr Double_t kColumnWidth = (1.0 - kColumnGap) / kColumnCount;
constexpr Double_t kColumnXMin[kColumnCount] = {
    0.0, kColumnWidth + kColumnGap
};
constexpr Double_t kColumnXMax[kColumnCount] = {
    kColumnWidth, 1.0
};
constexpr Int_t kXBins[kColumnCount] = {100, 100};
constexpr Double_t kXMinimum[kColumnCount] = {-10.0, 10.0};
constexpr Double_t kXMaximum[kColumnCount] = {10.0, 30.0};
constexpr Double_t kPadYMin[2] = {0.500, 0.000};
constexpr Double_t kPadYMax[2] = {1.000, 0.502};
constexpr Double_t kTopMargin[2] = {0.035, 0.000};
constexpr Double_t kBottomMargin[2] = {0.000, 0.145};
constexpr Double_t kLeftMargin[kColumnCount] = {0.14, 0.12};
constexpr Double_t kRightMargin[kColumnCount] = {0.015, 0.035};
constexpr Double_t kYTitleXFraction[kColumnCount] = {0.047, 0.020};
constexpr Double_t kPanelInset = 0.01;
constexpr Double_t kYTitleTextSize = 0.03;
const char *kBranchNames[kPanelCount] = {"Ex1", "Ex2", "Ex3", "Ex4"};

// 配置单个 pad 的行列相关边距、外观和双侧刻度。
void ConfigurePad(TPad *pad, Int_t columnIndex, Int_t rowIndex)
{
    pad->SetFillColor(kWhite);
    pad->SetFillStyle(1001);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);
    pad->SetLeftMargin(kLeftMargin[columnIndex]);
    pad->SetRightMargin(kRightMargin[columnIndex]);
    pad->SetTopMargin(kTopMargin[rowIndex]);
    pad->SetBottomMargin(kBottomMargin[rowIndex]);
    pad->SetTickx(1);
    pad->SetTicky(1);
}

// 配置直方图线型和坐标轴，仅在每列下图显示 X 轴标签及标题。
void ConfigureHistogram(TH1D *histogram, Int_t rowIndex)
{
    histogram->SetLineColor(kAzure - 4);
    histogram->SetLineWidth(1);
    histogram->SetMinimum(0.0);
    histogram->SetMaximum(1.18 * histogram->GetMaximum());

    histogram->GetXaxis()->SetNdivisions(506);
    histogram->GetXaxis()->SetTickLength(0.030);
    if (rowIndex == 1) {
        histogram->GetXaxis()->SetLabelFont(42);
        histogram->GetXaxis()->SetLabelSize(0.058);
        histogram->GetXaxis()->SetLabelOffset(0.012);
        histogram->GetXaxis()->SetTitle("E_{x} (MeV)");
        histogram->GetXaxis()->SetTitleFont(42);
        histogram->GetXaxis()->SetTitleSize(0.065);
        histogram->GetXaxis()->SetTitleOffset(1.00);
    } else {
        histogram->GetXaxis()->SetLabelSize(0.0);
        histogram->GetXaxis()->SetTitleSize(0.0);
    }

    histogram->GetYaxis()->SetNdivisions(505);
    histogram->GetYaxis()->SetLabelFont(42);
    histogram->GetYaxis()->SetLabelSize(0.060);
    histogram->GetYaxis()->SetTitle("");
    histogram->GetYaxis()->SetTickLength(0.018);
    histogram->GetYaxis()->SetLabelOffset(0.008);
    if (rowIndex == 1)
        histogram->GetYaxis()->ChangeLabel(-1, -1, 0.0);
    else
        histogram->GetYaxis()->ChangeLabel(1, -1, 0.0);
}

// 将一个 vector<double> 分支中的全部样本填入对应直方图。
void FillHistogram(TH1D *histogram, const std::vector<Double_t> &values)
{
    for (std::size_t index = 0; index < values.size(); ++index)
        histogram->Fill(values[index]);
}

// 按峰所在区域移动统计框，并绘制分支名和右上角面板编号。
void DrawPanelAnnotations(TPad *pad, TH1D *histogram, Int_t panelIndex)
{
    const Int_t columnIndex = panelIndex / 2;
    TPaveStats *stats =
        static_cast<TPaveStats *>(histogram->FindObject("stats"));
    if (stats) {
        if (columnIndex == 0) {
            stats->SetX1NDC(0.18);
            stats->SetX2NDC(0.44);
        } else {
            stats->SetX1NDC(0.68);
            stats->SetX2NDC(0.94);
        }
        stats->SetY1NDC(0.58);
        stats->SetY2NDC(0.90);
        stats->SetTextSize(0.032);
    }

    TLatex text;
    text.SetNDC();
    text.SetTextFont(42);
    text.SetTextSize(0.060);
    text.SetTextAlign(22);
    text.DrawLatex(0.56, 0.83, kBranchNames[panelIndex]);

    text.SetTextAlign(33);
    text.DrawLatex(1.0 - pad->GetRightMargin() - kPanelInset,
                   1.0 - pad->GetTopMargin() - kPanelInset,
                   Form("(%c)", 'a' + panelIndex));
}

// 在画布层为四幅图绘制由实际 bin width 生成的 Y 轴标题。
void DrawYAxisTitles(TH1D *histograms[kPanelCount])
{
    const Double_t titleY[2] = {0.75, 0.25};
    TLatex title;
    title.SetNDC();
    title.SetTextFont(42);
    title.SetTextSize(kYTitleTextSize);
    title.SetTextAlign(22);
    title.SetTextAngle(90);

    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        const Int_t columnIndex = panelIndex / 2;
        const Int_t rowIndex = panelIndex % 2;
        const Double_t titleX =
            kColumnXMin[columnIndex]
            + kYTitleXFraction[columnIndex] * kColumnWidth;
        const Double_t binWidth =
            histograms[panelIndex]->GetXaxis()->GetBinWidth(1);
        title.DrawLatex(titleX, titleY[rowIndex],
                        Form("Counts / %.3g MeV", binWidth));
    }
}

} // namespace

// 读取四个生成分支、绘制两列四面板直方图，并保存 PDF。
void DrawGeneratedFourPanels()
{
    TFile *inputFile = TFile::Open(kInputFile, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        Error("DrawGeneratedFourPanels", "无法打开 %s", kInputFile);
        return;
    }

    TTree *tree = nullptr;
    inputFile->GetObject("tree", tree);
    if (!tree) {
        Error("DrawGeneratedFourPanels", "文件中没有 tree");
        inputFile->Close();
        return;
    }

    std::vector<Double_t> *samples[kPanelCount] = {
        nullptr, nullptr, nullptr, nullptr
    };
    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex)
        tree->SetBranchAddress(kBranchNames[panelIndex], &samples[panelIndex]);
    tree->GetEntry(0);

    TH1D *h1 = new TH1D("h1", "", kXBins[0], kXMinimum[0], kXMaximum[0]);
    TH1D *h2 = new TH1D("h2", "", kXBins[0], kXMinimum[0], kXMaximum[0]);
    TH1D *h3 = new TH1D("h3", "", kXBins[1], kXMinimum[1], kXMaximum[1]);
    TH1D *h4 = new TH1D("h4", "", kXBins[1], kXMinimum[1], kXMaximum[1]);
    TH1D *histograms[kPanelCount] = {h1, h2, h3, h4};

    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        histograms[panelIndex]->SetDirectory(nullptr);
        FillHistogram(histograms[panelIndex], *samples[panelIndex]);
        ConfigureHistogram(histograms[panelIndex], panelIndex % 2);
    }

    gStyle->SetOptStat(111111111);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    gStyle->SetPaperSize(kPaperWidthCm, kPaperHeightCm);
    TGaxis::SetMaxDigits(4);

    TCanvas *c1 = new TCanvas("c1", "c1", kCanvasWidth, kCanvasHeight);
    c1->SetFillColor(kWhite);
    c1->SetBorderMode(0);

    TPad *pads[kPanelCount];
    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        const Int_t columnIndex = panelIndex / 2;
        const Int_t rowIndex = panelIndex % 2;
        pads[panelIndex] = new TPad(
            Form("p%d", panelIndex + 1), "",
            kColumnXMin[columnIndex], kPadYMin[rowIndex],
            kColumnXMax[columnIndex], kPadYMax[rowIndex]);
        ConfigurePad(pads[panelIndex], columnIndex, rowIndex);
        pads[panelIndex]->Draw();
    }

    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        pads[panelIndex]->cd();
        histograms[panelIndex]->Draw("HIST");
        pads[panelIndex]->Update();
        DrawPanelAnnotations(pads[panelIndex], histograms[panelIndex],
                             panelIndex);
        pads[panelIndex]->Modified();
        pads[panelIndex]->Update();
        pads[panelIndex]->RedrawAxis();
    }

    c1->cd();
    DrawYAxisTitles(histograms);
    c1->Modified();
    c1->Update();
    c1->SaveAs(kOutputFile);
    inputFile->Close();

    Info("DrawGeneratedFourPanels", "已保存 %s", kOutputFile);
}
