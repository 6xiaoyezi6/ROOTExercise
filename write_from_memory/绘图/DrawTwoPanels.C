/*
 * 功能：绘制纵向排列的两幅子图，并输出 TwoPanels.pdf；上图纵轴标题为 YLabel1，下图为 YLabel2。
 * 方法：上图使用 3 至 10^{3} 的对数 Y 轴，下图使用 0 至 1 的线性 Y 轴并固定主刻度间隔为 0.2。
 * 注意事项：下图顶端的数值 1 被主动隐藏，使两个子图的交界处保持空白。
 */

#include "TCanvas.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TPad.h"
#include "TStyle.h"

namespace {

// 配置单个子图的边距、外观、刻度和 Y 轴线性或对数模式。
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

    frame->GetXaxis()->SetRangeUser(0.0, 180.0);
    frame->GetXaxis()->SetNdivisions(506);
    frame->GetXaxis()->SetTickLength(0.030);

    if (showXAxis) {
        frame->GetXaxis()->SetLabelFont(42);
        frame->GetXaxis()->SetLabelSize(0.058);
        frame->GetXaxis()->SetLabelOffset(0.012);
        frame->GetXaxis()->SetTitle("#theta_{c.m.} (deg)");
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

// 在指定子图中绘制坐标框、能级文字和面板编号。
void DrawPanel(TPad *pad, TH1D *frame, const char *stateText,
               const char *panelText)
{
    pad->cd();
    frame->Draw("AXIS");

    // 在这里按需要为当前子图添加 TGraphErrors 数据和 TGraph 理论曲线。

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.070);
    latex.SetTextAlign(22);
    latex.DrawLatex(0.55, 0.72, stateText);
    latex.DrawLatex(0.87, 0.84, panelText);
    pad->RedrawAxis();
}

// 在画布左侧分别绘制上、下两个子图的 Y 轴标题。
void DrawYAxisTitles()
{
    TLatex title;
    title.SetNDC();
    title.SetTextFont(42);
    title.SetTextSize(0.050);
    title.SetTextAlign(22);
    title.SetTextAngle(90);
    title.DrawLatex(0.047, 0.75, "YLabel1");
    title.DrawLatex(0.047, 0.25, "YLabel2");
}

} // namespace

// 创建上下两幅子图、设置不同的 Y 轴范围和刻度模式，并保存 PDF。
void DrawTwoPanels()
{
    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    TGaxis::SetMaxDigits(3);

    TCanvas *canvas = new TCanvas("canvasTwoPanels", "Two panels", 650, 650);
    canvas->SetFillColor(kWhite);
    canvas->SetBorderMode(0);

    TPad *upperPad = new TPad("upperPad", "", 0.0, 0.500, 1.0, 1.000);
    TPad *lowerPad = new TPad("lowerPad", "", 0.0, 0.000, 1.0, 0.502);
    ConfigurePad(upperPad, 0.035, 0.000, kTRUE);
    ConfigurePad(lowerPad, 0.000, 0.145, kFALSE);

    upperPad->Draw();
    lowerPad->Draw();

    TH1D *upperFrame = new TH1D("upperFrame", "", 180, 0.0, 180.0);
    ConfigureFrame(upperFrame, kFALSE);
    upperFrame->SetMinimum(3.0);
    upperFrame->SetMaximum(1e3);
    upperFrame->GetYaxis()->SetNdivisions(505);
    DrawPanel(upperPad, upperFrame, "g.s.   0^{+}", "(a)");

    TH1D *lowerFrame = new TH1D("lowerFrame", "", 180, 0.0, 180.0);
    ConfigureFrame(lowerFrame, kTRUE);
    lowerFrame->SetMinimum(0.0);
    lowerFrame->SetMaximum(1.0);
    lowerFrame->GetYaxis()->SetNdivisions(505, kFALSE);
    lowerFrame->GetYaxis()->ChangeLabel(-1, -1, 0.0);
    DrawPanel(lowerPad, lowerFrame, "3.368 MeV   2^{+}", "(b)");

    canvas->cd();
    DrawYAxisTitles();
    canvas->Modified();
    canvas->Update();
    canvas->SaveAs("TwoPanels.pdf");
}
