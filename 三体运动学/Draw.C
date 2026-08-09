// Draw.C
//
// Read PureThreeBodyKinematics.root and draw fifteen PDF files.

#include <iostream>

#include "TAxis.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"
#include "TStyle.h"

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
    histogram->Draw("COLZ");
    canvas->SaveAs(outputFileName);
}

void SaveOneDimensionalHistogram(TCanvas* canvas,
                                 TH1D* histogram,
                                 const char* outputFileName)
{
    canvas->Clear();
    histogram->Draw();
    canvas->SaveAs(outputFileName);
}

void Draw()
{
    gStyle->SetOptStat(111111111);

    const char* InputFileName = "PureThreeBodyKinematics.root";

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

    TGraph* gB_Tmin =
        (TGraph*)histogramDirectory->Get("gB_Tmin");
    TGraph* gB_Tmax =
        (TGraph*)histogramDirectory->Get("gB_Tmax");
    TGraph* g1_Tmin =
        (TGraph*)histogramDirectory->Get("g1_Tmin");
    TGraph* g1_Tmax =
        (TGraph*)histogramDirectory->Get("g1_Tmax");
    TGraph* g2_Tmin =
        (TGraph*)histogramDirectory->Get("g2_Tmin");
    TGraph* g2_Tmax =
        (TGraph*)histogramDirectory->Get("g2_Tmax");

    const double ThetaDisplayMin = 0.0;
    const double ThetaDisplayMax = 90.0;

    hB_TTheta->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);
    h1_TTheta->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);
    h2_TTheta->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);

    hThetaB_Theta1->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);
    hThetaB_Theta1->GetYaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);

    hThetaB_Theta2->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);
    hThetaB_Theta2->GetYaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);

    hTheta1_Theta2->GetXaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);
    hTheta1_Theta2->GetYaxis()->SetRangeUser(ThetaDisplayMin, ThetaDisplayMax);

    hDeltaPhiB1->GetXaxis()->SetRangeUser(0.0, 360.0);
    hDeltaPhiB2->GetXaxis()->SetRangeUser(0.0, 360.0);
    hDeltaPhi12->GetXaxis()->SetRangeUser(0.0, 360.0);

    SetHistAxisTitle(hDeltaPhiB1, "#Delta#phi_{B1}^{lab}", "deg");
    SetHistAxisTitle(hDeltaPhiB2, "#Delta#phi_{B2}^{lab}", "deg");
    SetHistAxisTitle(hDeltaPhi12, "#Delta#phi_{12}^{lab}", "deg");

    TCanvas* canvas = new TCanvas("canvas", "Three-body kinematics", 900, 700);

    canvas->Clear();
    hB_TTheta->Draw("COLZ");
    gB_Tmin->Draw("L SAME");
    gB_Tmax->Draw("L SAME");
    canvas->SaveAs("PureThreeBody_B_T_vs_theta.pdf");

    canvas->Clear();
    h1_TTheta->Draw("COLZ");
    g1_Tmin->Draw("L SAME");
    g1_Tmax->Draw("L SAME");
    canvas->SaveAs("PureThreeBody_1_T_vs_theta.pdf");

    canvas->Clear();
    h2_TTheta->Draw("COLZ");
    g2_Tmin->Draw("L SAME");
    g2_Tmax->Draw("L SAME");
    canvas->SaveAs("PureThreeBody_2_T_vs_theta.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hT1_T2, "PureThreeBody_T1_vs_T2.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTB_T1, "PureThreeBody_TB_vs_T1.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTB_T2, "PureThreeBody_TB_vs_T2.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hThetaB_Theta1, "PureThreeBody_thetaB_vs_theta1.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hThetaB_Theta2, "PureThreeBody_thetaB_vs_theta2.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hTheta1_Theta2, "PureThreeBody_theta1_vs_theta2.pdf");

    SaveTwoDimensionalHistogram(
        canvas, hPhiB_Phi1, "PureThreeBody_phiB_vs_phi1.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hPhiB_Phi2, "PureThreeBody_phiB_vs_phi2.pdf");
    SaveTwoDimensionalHistogram(
        canvas, hPhi1_Phi2, "PureThreeBody_phi1_vs_phi2.pdf");

    SaveOneDimensionalHistogram(
        canvas, hDeltaPhiB1, "PureThreeBody_deltaPhiB1.pdf");
    SaveOneDimensionalHistogram(
        canvas, hDeltaPhiB2, "PureThreeBody_deltaPhiB2.pdf");
    SaveOneDimensionalHistogram(
        canvas, hDeltaPhi12, "PureThreeBody_deltaPhi12.pdf");

    inputFile->Close();

    cout << "Fifteen PDF files were written successfully." << endl;
}
