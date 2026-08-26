/*
 * 功能：绘制两列两行的四幅子图并输出 FourPanels.pdf；每列上下两图共享横轴配置，四幅图的纵轴范围可独立设置。
 * 方法：用四个 TPad 组成左右两份纵向双子图，按列设置边距并保留可调间隙；公共函数统一坐标框、文字和面板编号。
 * 注意事项：坐标配置、画布与纸张尺寸、标题字号及列间隙集中在顶部常量中；下排顶端 Y 标签会被隐藏，运行时覆盖同名 PDF。
 */

#include "TCanvas.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TPad.h"
#include "TString.h"
#include "TStyle.h"

namespace {

constexpr Int_t kPanelCount = 4;
constexpr Int_t kColumnCount = 2;
constexpr Int_t kCanvasWidth = 1300;
constexpr Int_t kCanvasHeight = 800;
constexpr Double_t kPaperWidthCm = 26.0;
constexpr Double_t kPaperHeightCm = 16.0;
constexpr Double_t kColumnGap = 0.01;//控制左右列间距
constexpr Double_t kColumnWidth = (1.0 - kColumnGap) / kColumnCount;
constexpr Double_t kColumnXMin[kColumnCount] = {
    0.0, kColumnWidth + kColumnGap
};
constexpr Double_t kColumnXMax[kColumnCount] = {
    kColumnWidth, 1.0
};

// 每一列只使用一套 X 轴配置，保证该列上下两图的横轴完全一致。
constexpr Int_t kXBins[kColumnCount] = {180, 180};
constexpr Double_t kXMinimum[kColumnCount] = {0.0, 0.0};
constexpr Double_t kXMaximum[kColumnCount] = {180.0, 180.0};
const char *kXTitle[kColumnCount] = {
    "#theta_{c.m.} (deg)",
    "#theta_{c.m.} (deg)"
};

// 四幅图的 Y 轴范围和线性/对数模式彼此独立，可按需要分别修改。
constexpr Double_t kYMinimum[kPanelCount] = {3.0, 0.0, 3.0, 0.0};
constexpr Double_t kYMaximum[kPanelCount] = {1e3, 1.0, 1e3, 1.0};
constexpr Bool_t kLogY[kPanelCount] = {kTRUE, kFALSE, kTRUE, kFALSE};
constexpr Int_t kYDivisions[kPanelCount] = {505, 505, 505, 505};
constexpr Bool_t kOptimizeYDivisions[kPanelCount] = {
    kTRUE, kFALSE, kTRUE, kFALSE
};
const char *kYTitle[kPanelCount] = {
    "YLabel1", "YLabel2", "YLabel3", "YLabel4"
};
const char *kStateText[kPanelCount] = {
    "g.s.   0^{+}",
    "3.368 MeV   2^{+}",
    "g.s.   0^{+}",
    "3.368 MeV   2^{+}"
};

constexpr Double_t kPadYMin[2] = {0.500, 0.000};
constexpr Double_t kPadYMax[2] = {1.000, 0.502};
constexpr Double_t kTopMargin[2] = {0.035, 0.000};
constexpr Double_t kBottomMargin[2] = {0.000, 0.145};
constexpr Double_t kLeftMargin[kColumnCount] = {0.14, 0.12};
constexpr Double_t kRightMargin[kColumnCount] = {0.015, 0.035};
constexpr Double_t kYTitleXFraction[kColumnCount] = {0.047, 0.020};
constexpr Double_t kPanelInset = 0.01;
constexpr Double_t kYTitleTextSize = 0.03;

// 配置单个子图的边距、外观、刻度和 Y 轴线性或对数模式。
void ConfigurePad(TPad *pad, Double_t topMargin, Double_t bottomMargin,
                  Bool_t logY, Int_t columnIndex)
{
    pad->SetFillColor(kWhite);
    pad->SetFillStyle(1001);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);
    pad->SetLeftMargin(kLeftMargin[columnIndex]);
    pad->SetRightMargin(kRightMargin[columnIndex]);
    pad->SetTopMargin(topMargin);
    pad->SetBottomMargin(bottomMargin);
    pad->SetLogy(logY);
    pad->SetTickx(1);
    pad->SetTicky(1);
}

// 配置坐标框，并保证同一列上下两图使用相同的 X 轴范围、分度和标题。
void ConfigureFrame(TH1D *frame, Int_t columnIndex, Bool_t showXAxis)
{
    frame->SetLineWidth(2);
    frame->GetXaxis()->SetRangeUser(kXMinimum[columnIndex],
                                    kXMaximum[columnIndex]);
    frame->GetXaxis()->SetNdivisions(506);
    frame->GetXaxis()->SetTickLength(0.030);

    if (showXAxis) {
        frame->GetXaxis()->SetLabelFont(42);
        frame->GetXaxis()->SetLabelSize(0.058);
        frame->GetXaxis()->SetLabelOffset(0.012);
        frame->GetXaxis()->SetTitle(kXTitle[columnIndex]);
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

// 在指定子图中绘制坐标框、状态文字和右上角的顺序面板编号。
void DrawPanel(TPad *pad, TH1D *frame, Int_t panelIndex)
{
    pad->cd();
    frame->Draw("AXIS");

    // 在这里按需要为当前子图添加 TGraphErrors 数据和 TGraph 理论曲线。

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.070);
    latex.SetTextAlign(22);
    latex.DrawLatex(0.55, 0.72, kStateText[panelIndex]);

    latex.SetTextAlign(33);
    latex.DrawLatex(1.0 - pad->GetRightMargin() - kPanelInset,
                    1.0 - pad->GetTopMargin() - kPanelInset,
                    Form("(%c)", 'a' + panelIndex));
    pad->RedrawAxis();
}

// 在画布层分别绘制四幅子图的 Y 轴标题。
void DrawYAxisTitles()
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
        title.DrawLatex(titleX, titleY[rowIndex], kYTitle[panelIndex]);
    }
}

} // namespace

// 创建两列两行子图、应用各自 Y 轴配置，并保存 FourPanels.pdf。
void DrawFourPanels()
{
    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLineWidth(2);
    gStyle->SetPaperSize(kPaperWidthCm, kPaperHeightCm);
    TGaxis::SetMaxDigits(3);

    TCanvas *c1 = new TCanvas("c1", "c1", kCanvasWidth, kCanvasHeight);
    c1->SetFillColor(kWhite);
    c1->SetBorderMode(0);

    TPad *pads[kPanelCount];
    TH1D *frames[kPanelCount];

    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        const Int_t columnIndex = panelIndex / 2;
        const Int_t rowIndex = panelIndex % 2;
        pads[panelIndex] = new TPad(
            Form("p%d", panelIndex + 1), "",
            kColumnXMin[columnIndex], kPadYMin[rowIndex],
            kColumnXMax[columnIndex], kPadYMax[rowIndex]);
        ConfigurePad(pads[panelIndex], kTopMargin[rowIndex],
                     kBottomMargin[rowIndex], kLogY[panelIndex],
                     columnIndex);
        pads[panelIndex]->Draw();
    }

    for (Int_t panelIndex = 0; panelIndex < kPanelCount; ++panelIndex) {
        const Int_t columnIndex = panelIndex / 2;
        const Int_t rowIndex = panelIndex % 2;
        frames[panelIndex] = new TH1D(
            Form("h%d", panelIndex + 1), "", kXBins[columnIndex],
            kXMinimum[columnIndex], kXMaximum[columnIndex]);
        ConfigureFrame(frames[panelIndex], columnIndex, rowIndex == 1);
        frames[panelIndex]->SetMinimum(kYMinimum[panelIndex]);
        frames[panelIndex]->SetMaximum(kYMaximum[panelIndex]);
        frames[panelIndex]->GetYaxis()->SetNdivisions(
            kYDivisions[panelIndex], kOptimizeYDivisions[panelIndex]);

        if (rowIndex == 1)
            frames[panelIndex]->GetYaxis()->ChangeLabel(-1, -1, 0.0);

        DrawPanel(pads[panelIndex], frames[panelIndex], panelIndex);
    }

    c1->cd();
    DrawYAxisTitles();
    c1->Modified();
    c1->Update();
    c1->SaveAs("FourPanels.pdf");
}
