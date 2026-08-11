#include <cmath>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TF2.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TObjArray.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TSystem.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/constants.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"


Double_t ThreeBodyBoundaryEquation(Double_t* x, Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t theta1 = x[0] * TMath::DegToRad();
    const Double_t T1 = x[1];
    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];

    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t p1 = std::sqrt(T1 * (T1 + 2.0 * m1));

    const Double_t leftSide =
        (mA + TA + mT) * (T1 + m1)
        - pA * p1 * std::cos(theta1);

    const Double_t rightSide =
        0.5 * (mA * mA + mT * mT + m1 * m1
               - (mB + m2) * (mB + m2))
        + mT * (mA + TA);

    return leftSide - rightSide;
}


void ThreeBodyBoundary()
{

    gStyle->SetOptStat(0);
    gSystem->mkdir("output", kTRUE);
    // 10Be + p -> 6He + p + alpha
    const Double_t massA = 10.0135338;       // u
    const Double_t massT = 1.007276466621;   // u
    const Double_t massB = 6.01888589;       // u
    const Double_t mass1 = 1.007276466621;   // u
    const Double_t mass2 = 4.001506179127;   // u
    const Double_t beamEnergyPerU = 150.0;   // MeV/u
    const Double_t massNumberA = 10.0;

    TFile* inputFile = TFile::Open(
        "output/PureThreeBodyKinematics.root",
        "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cout << "[ERROR] Cannot open output/PureThreeBodyKinematics.root"
                  << std::endl;
        return;
    }

    TDirectory* histogramDirectory =
        (TDirectory*)inputFile->Get("histograms");
    TH2D* h1TTheta =
        (TH2D*)histogramDirectory->Get("h1_TTheta");
    TGraph* generatedMinimum =
        (TGraph*)histogramDirectory->Get("g1_Tmin");
    TGraph* generatedMaximum =
        (TGraph*)histogramDirectory->Get("g1_Tmax");

    const Double_t thetaMin = 0.0;
    const Double_t thetaMax = 90.0;
    const Double_t energyMin = h1TTheta->GetYaxis()->GetXmin();
    const Double_t energyMax = h1TTheta->GetYaxis()->GetXmax();

    h1TTheta->GetXaxis()->SetRangeUser(thetaMin, thetaMax);

    TF2* boundary = new TF2(
        "threeBodyBoundary",
        ThreeBodyBoundaryEquation,
        thetaMin,
        thetaMax,
        energyMin,
        energyMax,
        7);
    boundary->SetParameters(
        massA,
        massT,
        massB,
        mass1,
        mass2,
        beamEnergyPerU,
        massNumberA);
    boundary->SetNpx(1200);
    boundary->SetNpy(1200);
    boundary->SetLineColor(kBlack);
    boundary->SetLineStyle(kSolid);
    boundary->SetLineWidth(2);

    Double_t contourLevel[1] = {0.0};
    boundary->SetContour(1, contourLevel);

    TCanvas* canvas = new TCanvas(
        "canvasThreeBodyBoundary",
        "Three-body kinematic boundary",
        900,
        700);

    boundary->Draw("CONT LIST");
    canvas->Update();

    TObjArray* contours =
        (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
    TList* zeroContour = (TList*)contours->At(0);
    std::vector<TGraph*> boundaryGraphs;

    for (Int_t i = 0; i < zeroContour->GetSize(); ++i) {
        TGraph* sourceGraph = (TGraph*)zeroContour->At(i);
        TGraph* boundaryGraph = (TGraph*)sourceGraph->Clone();
        boundaryGraph->SetLineColor(kBlack);
        boundaryGraph->SetLineStyle(kSolid);
        boundaryGraph->SetLineWidth(2);
        boundaryGraphs.push_back(boundaryGraph);
    }

    TMultiGraph* alphaProtonElastic =
        TwoBodyKinematicsEnergyAngleProton(
            150.0,
            0.0,
            tof_constants::m_4He_AMU,
            tof_constants::m_1H_AMU,
            tof_constants::m_1H_AMU,
            tof_constants::m_4He_AMU,
            kGreen + 2,
            2,
            kSolid);

    TGraph* alphaProtonPhysicalBranch =
        (TGraph*)alphaProtonElastic->GetListOfGraphs()->First();

    canvas->Clear();
    h1TTheta->Draw("COLZ");
    generatedMinimum->Draw("L SAME");
    generatedMaximum->Draw("L SAME");

    for (Int_t i = 0; i < (Int_t)boundaryGraphs.size(); ++i) {
        boundaryGraphs[i]->Draw("L SAME");
    }

    alphaProtonPhysicalBranch->Draw("L SAME");

    TLegend* legend = new TLegend(0.16, 0.73, 0.54, 0.87);
    legend->SetBorderSize(0);
    legend->AddEntry(
        boundaryGraphs[0],
        "Implicit boundary: M_{B2}=m_{B}+m_{2}",
        "l");
    legend->AddEntry(
        alphaProtonPhysicalBranch,
        "^{4}He+p elastic, T_{#alpha}=150 MeV/u",
        "l");
    legend->Draw();

    canvas->SaveAs(
        "output/PureThreeBody_1_T_vs_theta_with_boundary.pdf");

    inputFile->Close();
}
