// Draw.C
//
// 功能：读取 output/PureThreeBodyKinematics.root，将末态粒子动能从总动能
//       MeV 换算为 MeV/u，并绘制 15 张无顶部标题、无统计框的三体运动学 PDF 图。
// 方法：按 B(6He)、1(p)、2(alpha) 的质量数 6、1、4 线性缩放直方图动能坐标，
//       能量-角度图保持角度为横轴，含 T1 的动能-动能图保持 T1 为横轴；
//       PureThreeBodyTheta1Theta2.pdf 以 theta2 为横轴、theta1 为纵轴；
//       只绘制蒙特卡洛直方图，不在本文件中叠加运动学边界。
// 注意事项：输入 ROOT 文件中的动能必须为总动能 MeV；若更换反应道或粒子同位素，
//           必须同步修改 Draw() 中的三个质量数常量；运动学边界由独立宏绘制。

#include <iostream>

#include "TAxis.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"

using namespace std;

void SetHistAxisTitle(TH1* histogram,
                      const char* xTitle,
                      const char* xUnit)
{
    double binWidth = histogram->GetXaxis()->GetBinWidth(1);

    histogram->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
    histogram->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void SaveTwoDimensionalHistogram(TCanvas* canvas,
                                 TH2D* histogram,
                                 const char* outputFileName)
{
    canvas->Clear();
    histogram->SetTitle("");
    histogram->Draw("COLZ");
    canvas->SaveAs(outputFileName);
}

void SaveOneDimensionalHistogram(TCanvas* canvas,
                                 TH1D* histogram,
                                 const char* outputFileName)
{
    canvas->Clear();
    histogram->SetTitle("");
    histogram->Draw();
    canvas->SaveAs(outputFileName);
}

void ScaleAxisToEnergyPerNucleon(TAxis* axis, double massNumber)
{
    axis->SetLimits(axis->GetXmin() / massNumber,
                    axis->GetXmax() / massNumber);
}

void Draw()
{
    gStyle->SetOptStat(0);
    gSystem->mkdir("output", kTRUE);

    const char* InputFileName = "output/PureThreeBodyKinematics.root";

    TFile* inputFile = TFile::Open(InputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        cout << "[ERROR] Cannot open " << InputFileName << endl;
        return;
    }

    TDirectory* histogramDirectory =
        (TDirectory*)inputFile->Get("histograms");

    if (!histogramDirectory) {
        cout << "[ERROR] Cannot find histograms directory in "
             << InputFileName << endl;
        inputFile->Close();
        return;
    }

    TH2D* hB_TTheta =
        (TH2D*)histogramDirectory->Get("hB_TTheta");
    TH2D* h1_TTheta =
        (TH2D*)histogramDirectory->Get("h1_TTheta");
    TH2D* h2_TTheta =
        (TH2D*)histogramDirectory->Get("h2_TTheta");

    TH2D* hT1_T2 =
        (TH2D*)histogramDirectory->Get("hT1_T2");
    TH2D* hTB_T1 =
        (TH2D*)histogramDirectory->Get("hTB_T1");
    TH2D* hTB_T2 =
        (TH2D*)histogramDirectory->Get("hTB_T2");

    TH2D* hThetaB_Theta1 =
        (TH2D*)histogramDirectory->Get("hThetaB_Theta1");
    TH2D* hThetaB_Theta2 =
        (TH2D*)histogramDirectory->Get("hThetaB_Theta2");
    TH2D* hTheta1_Theta2 =
        (TH2D*)histogramDirectory->Get("hTheta1_Theta2");

    TH2D* hPhiB_Phi1 =
        (TH2D*)histogramDirectory->Get("hPhiB_Phi1");
    TH2D* hPhiB_Phi2 =
        (TH2D*)histogramDirectory->Get("hPhiB_Phi2");
    TH2D* hPhi1_Phi2 =
        (TH2D*)histogramDirectory->Get("hPhi1_Phi2");

    TH1D* hDeltaPhiB1 =
        (TH1D*)histogramDirectory->Get("hDeltaPhiB1");
    TH1D* hDeltaPhiB2 =
        (TH1D*)histogramDirectory->Get("hDeltaPhiB2");
    TH1D* hDeltaPhi12 =
        (TH1D*)histogramDirectory->Get("hDeltaPhi12");

    const double MassNumberB = 6.0;
    const double MassNumber1 = 1.0;
    const double MassNumber2 = 4.0;

    ScaleAxisToEnergyPerNucleon(hB_TTheta->GetYaxis(), MassNumberB);
    ScaleAxisToEnergyPerNucleon(h1_TTheta->GetYaxis(), MassNumber1);
    ScaleAxisToEnergyPerNucleon(h2_TTheta->GetYaxis(), MassNumber2);

    hB_TTheta->GetYaxis()->SetTitle("T_{B}^{lab} [MeV/u]");
    h1_TTheta->GetYaxis()->SetTitle("T_{1}^{lab} [MeV/u]");
    h2_TTheta->GetYaxis()->SetTitle("T_{2}^{lab} [MeV/u]");

    ScaleAxisToEnergyPerNucleon(hT1_T2->GetXaxis(), MassNumber1);
    ScaleAxisToEnergyPerNucleon(hT1_T2->GetYaxis(), MassNumber2);
    hT1_T2->GetXaxis()->SetTitle("T_{1}^{lab} [MeV/u]");
    hT1_T2->GetYaxis()->SetTitle("T_{2}^{lab} [MeV/u]");

    ScaleAxisToEnergyPerNucleon(hTB_T1->GetXaxis(), MassNumber1);
    ScaleAxisToEnergyPerNucleon(hTB_T1->GetYaxis(), MassNumberB);
    hTB_T1->GetXaxis()->SetTitle("T_{1}^{lab} [MeV/u]");
    hTB_T1->GetYaxis()->SetTitle("T_{B}^{lab} [MeV/u]");

    ScaleAxisToEnergyPerNucleon(hTB_T2->GetXaxis(), MassNumberB);
    ScaleAxisToEnergyPerNucleon(hTB_T2->GetYaxis(), MassNumber2);
    hTB_T2->GetXaxis()->SetTitle("T_{B}^{lab} [MeV/u]");
    hTB_T2->GetYaxis()->SetTitle("T_{2}^{lab} [MeV/u]");

    const double T1DisplayMin = 0.0;
    const double T1DisplayMax = 800.0;
    const double ThetaBDisplayMin = 0.0;
    const double ThetaBDisplayMax = 40.0;
    const double Theta1DisplayMin = 0.0;
    const double Theta1DisplayMax = 90.0;
    const double Theta2DisplayMin = 0.0;
    const double Theta2DisplayMax = 40.0;

    h1_TTheta->GetYaxis()->SetRangeUser(T1DisplayMin, T1DisplayMax);
    hT1_T2->GetXaxis()->SetRangeUser(T1DisplayMin, T1DisplayMax);
    hTB_T1->GetXaxis()->SetRangeUser(T1DisplayMin, T1DisplayMax);

    hB_TTheta->GetXaxis()->SetRangeUser(ThetaBDisplayMin, ThetaBDisplayMax);
    h1_TTheta->GetXaxis()->SetRangeUser(Theta1DisplayMin, Theta1DisplayMax);
    h2_TTheta->GetXaxis()->SetRangeUser(Theta2DisplayMin, Theta2DisplayMax);

    hThetaB_Theta1->GetXaxis()->SetRangeUser(ThetaBDisplayMin,
                                             ThetaBDisplayMax);
    hThetaB_Theta1->GetYaxis()->SetRangeUser(Theta1DisplayMin,
                                             Theta1DisplayMax);

    hThetaB_Theta2->GetXaxis()->SetRangeUser(ThetaBDisplayMin,
                                             ThetaBDisplayMax);
    hThetaB_Theta2->GetYaxis()->SetRangeUser(Theta2DisplayMin,
                                             Theta2DisplayMax);

    hTheta1_Theta2->GetXaxis()->SetRangeUser(Theta2DisplayMin,
                                             Theta2DisplayMax);
    hTheta1_Theta2->GetYaxis()->SetRangeUser(Theta1DisplayMin,
                                             Theta1DisplayMax);

    hDeltaPhiB1->GetXaxis()->SetRangeUser(0.0, 360.0);
    hDeltaPhiB2->GetXaxis()->SetRangeUser(0.0, 360.0);
    hDeltaPhi12->GetXaxis()->SetRangeUser(0.0, 360.0);

    SetHistAxisTitle(hDeltaPhiB1, "#Delta#phi_{B1}^{lab}", "deg");
    SetHistAxisTitle(hDeltaPhiB2, "#Delta#phi_{B2}^{lab}", "deg");
    SetHistAxisTitle(hDeltaPhi12, "#Delta#phi_{12}^{lab}", "deg");

    TCanvas* canvas = new TCanvas("canvas", "Three-body kinematics", 900, 700);

    canvas->Clear();
    hB_TTheta->SetTitle("");
    hB_TTheta->Draw("COLZ");
    canvas->SaveAs("output/PureThreeBodyTTheta_B.pdf");

    canvas->Clear();
    h1_TTheta->SetTitle("");
    h1_TTheta->Draw("COLZ");
    canvas->SaveAs("output/PureThreeBodyTTheta_1.pdf");

    canvas->Clear();
    h2_TTheta->SetTitle("");
    h2_TTheta->Draw("COLZ");
    canvas->SaveAs("output/PureThreeBodyTTheta_2.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hT1_T2, "output/PureThreeBodyT1T2.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTB_T1, "output/PureThreeBodyT1TB.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTB_T2, "output/PureThreeBodyT2TB.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hThetaB_Theta1, "output/PureThreeBodyTheta1ThetaB.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hThetaB_Theta2, "output/PureThreeBodyTheta2ThetaB.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTheta1_Theta2, "output/PureThreeBodyTheta1Theta2.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hPhiB_Phi1, "output/PureThreeBodyPhi1PhiB.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hPhiB_Phi2, "output/PureThreeBodyPhi2PhiB.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hPhi1_Phi2, "output/PureThreeBodyPhi1Phi2.pdf");

    SaveOneDimensionalHistogram(
        canvas, hDeltaPhiB1, "output/PureThreeBody_deltaPhiB1.pdf");
    SaveOneDimensionalHistogram(
        canvas, hDeltaPhiB2, "output/PureThreeBody_deltaPhiB2.pdf");
    SaveOneDimensionalHistogram(
        canvas, hDeltaPhi12, "output/PureThreeBody_deltaPhi12.pdf");

    inputFile->Close();

    cout << "Fifteen PDF files were written successfully." << endl;
}
