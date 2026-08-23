/*
 * 功能：绘制纵向排列的三幅共享坐标风格子图，并输出 ThreePanels.pdf。
 * 方法：用数组和公共配置函数统一设置 TPad、TH1D 坐标框及面板文字；三幅图均采用对数 Y 轴。
 * 注意事项：交界处的自动端点标签会被 pad 边界裁剪，因此改由 canvas 层统一绘制两个完整的 10^{2}。
 */

#include "TCanvas.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TPad.h"
#include "TStyle.h"

namespace {

void ConfigurePad(TPad *pad, Double_t topMargin, Double_t bottomMargin)
{
    pad->SetFillColor(kWhite);
    pad->SetFillStyle(1001);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);
    pad->SetLeftMargin(0.16);
    pad->SetRightMargin(0.035);
    pad->SetTopMargin(topMargin);
    pad->SetBottomMargin(bottomMargin);
    pad->SetLogy();
    pad->SetTickx(1);
    pad->SetTicky(1);
}

void ConfigureFrame(TH1D *frame, Bool_t showXAxis,
                    Bool_t hideBottomYLabel, Bool_t hideTopYLabel)
{
    frame->SetMinimum(1e-3);
    frame->SetMaximum(1e2);
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

    frame->GetYaxis()->SetNdivisions(505);
    frame->GetYaxis()->SetLabelFont(42);
    frame->GetYaxis()->SetLabelSize(0.060);
    frame->GetYaxis()->SetTitle("");
    frame->GetYaxis()->SetTickLength(0.018);
    frame->GetYaxis()->SetLabelOffset(0.008);

    // 隐藏交界两侧的自动端点标签，稍后在 canvas 层统一绘制 10^{2}。
    if (hideBottomYLabel)
        frame->GetYaxis()->ChangeLabel(1, -1, 0.0);
    if (hideTopYLabel)
        frame->GetYaxis()->ChangeLabel(-1, -1, 0.0);
}

void DrawSharedYLabels()
{
    TLatex label;
    label.SetNDC();
    label.SetTextFont(42);
    label.SetTextSize(0.027);
    label.SetTextAlign(32);
    label.DrawLatex(0.146, 0.667, "10^{2}");
    label.DrawLatex(0.146, 0.332, "10^{2}");
}

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

} // namespace

void DrawThreePanels()
{
    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    TGaxis::SetMaxDigits(3);

    TCanvas *canvas = new TCanvas("canvas", "Three panels", 650, 900);
    canvas->SetFillColor(kWhite);
    canvas->SetBorderMode(0);

    const Double_t padYMin[3] = {0.665, 0.330, 0.000};
    const Double_t padYMax[3] = {1.000, 0.667, 0.332};
    const Double_t topMargin[3] = {0.035, 0.000, 0.000};
    const Double_t bottomMargin[3] = {0.000, 0.000, 0.145};
    const char *stateText[3] = {
        "g.s.   0^{+}",
        "3.368 MeV   2^{+}",
        "7.27(10) MeV   1^{-}"
    };
    const char *panelText[3] = {"(a)", "(b)", "(c)"};

    TPad *pads[3];
    TH1D *frames[3];

    for (Int_t i = 0; i < 3; ++i) {
        pads[i] = new TPad(Form("pad%d", i + 1), "", 0.0, padYMin[i], 1.0, padYMax[i]);
        ConfigurePad(pads[i], topMargin[i], bottomMargin[i]);
    }

    // 从上到下绘制，使下方 pad 的公共边界框线位于最上层；轻微重叠可避免 PDF 细白缝。
    for (Int_t i = 0; i < 3; ++i)
        pads[i]->Draw();

    for (Int_t i = 0; i < 3; ++i) {
        frames[i] = new TH1D(Form("frame%d", i + 1), "", 180, 0.0, 180.0);
        ConfigureFrame(frames[i], i == 2, i < 2, i > 0);
        DrawPanel(pads[i], frames[i], stateText[i], panelText[i]);
    }

    canvas->cd();
    TLatex yTitle;
    yTitle.SetNDC();
    yTitle.SetTextFont(42);
    yTitle.SetTextSize(0.045);
    yTitle.SetTextAlign(22);
    yTitle.SetTextAngle(90);
    yTitle.DrawLatex(0.047, 0.50, "d#sigma/d#Omega (mb/sr)");
    DrawSharedYLabels();

    canvas->Modified();
    canvas->Update();
    canvas->SaveAs("ThreePanels.pdf");
}
