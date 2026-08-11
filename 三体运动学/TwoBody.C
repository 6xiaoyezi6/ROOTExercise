#include <cmath>

#include "TCanvas.h"
#include "TF2.h"
#include "TMath.h"
#include "TString.h"
#include "TSystem.h"

//用 TF2 + 等高线（contour）画出来隐函数运动学曲线
Double_t Kinematics(Double_t* x, Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t thetaC = x[0] * TMath::DegToRad();
    const Double_t ma = par[0] * AmuToMeV;
    const Double_t mb = par[1] * AmuToMeV;
    const Double_t mc = par[2] * AmuToMeV;
    const Double_t md = par[3] * AmuToMeV;
    const Double_t Ta = par[4] * par[5];
    const Double_t Tc = x[1] * par[6];

    const Double_t Ea = Ta + ma;
    const Double_t Ec = Tc + mc;
    const Double_t pa = std::sqrt(Ta * Ta + 2.0 * ma * Ta);
    const Double_t pc = std::sqrt(Tc * Tc + 2.0 * mc * Tc);

    const Double_t leftSide =
        (Ea + mb) * Ec - pa * pc * std::cos(thetaC);
    const Double_t rightSide =
        0.5 * (ma * ma + mb * mb + mc * mc - md * md)
        + Ea * mb;

    return leftSide - rightSide;
}


void TwoBody()
{
    gSystem->mkdir("output", kTRUE);

    // a + b -> c + d : 4He + p -> p + 4He
    const Double_t ma = 4.001506179127;   // u
    const Double_t mb = 1.007276466621;   // u
    const Double_t mc = 1.007276466621;   // u
    const Double_t md = 4.001506179127;   // u
    const Double_t TaPerU = 150.0;        // MeV/u
    const Double_t Aa = 4.0;
    const Double_t Ac = 1.0;

    constexpr Double_t AmuToMeV = 931.49410242;
    const Double_t qValue = (ma + mb - mc - md) * AmuToMeV;
    const Double_t TcMax = 1.05 * (TaPerU * Aa + qValue) / Ac;

    TF2* curve = new TF2("curve",Kinematics,0.0,90.0,1.0e-3,TcMax,7);

    curve->SetParameters(ma, mb, mc, md, TaPerU, Aa, Ac);
    curve->SetNpx(1000);
    curve->SetNpy(1000);

    Double_t contourLevel[1] = {0.0};
    curve->SetContour(1, contourLevel);
    curve->SetLineWidth(3);
    curve->SetTitle(Form(
        "^{4}He+p #rightarrow p+^{4}He, T_{#alpha}=%.1f MeV/u;"
        "#theta_{p}^{lab} [deg];T_{p} [MeV/u]",
        TaPerU));

    TCanvas* canvas = new TCanvas(
        "canvas",
        "Two-body kinematics",
        900,
        700);
    canvas->SetLeftMargin(0.13);
    canvas->SetBottomMargin(0.13);
    canvas->SetRightMargin(0.05);
    canvas->SetTopMargin(0.08);

    curve->GetXaxis()->CenterTitle();
    curve->GetYaxis()->CenterTitle();
    curve->GetXaxis()->SetTitleSize(0.050);
    curve->GetYaxis()->SetTitleSize(0.050);
    curve->GetXaxis()->SetLabelSize(0.045);
    curve->GetYaxis()->SetLabelSize(0.045);
    curve->GetXaxis()->SetTitleOffset(1.1);
    curve->GetYaxis()->SetTitleOffset(1.2);

    curve->Draw("CONT1");
    canvas->SaveAs("output/twobody.pdf");
}
