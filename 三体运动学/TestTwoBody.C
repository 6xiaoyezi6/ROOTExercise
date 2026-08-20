// TestTwoBody.C
//
// 功能：比较 10Be+p 基态弹性散射的隐式相对论二体运动学曲线与
//       TwoBodyKinematics 工具返回的质子能量-角度曲线，输出对比 PDF。
// 方法：由能量和动量守恒构造 TF2 零等高线，再叠加
//       TwoBodyKinematicsEnergyAngleProton 给出的物理分支。
// 注意事项：默认入射能量为 150 MeV/u，输出能量单位为 MeV/u；依赖本机
//           TwoBodyKinematics 绝对路径，并会覆盖 output 目录中的同名 PDF。

#include <cmath>

#include "TCanvas.h"
#include "TF2.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/constants.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"


Double_t ImplicitKinematics(Double_t* x, Double_t* par)
{
    const Double_t thetaC = x[0] * TMath::DegToRad();
    const Double_t ma = par[0] * tof_constants::amu;
    const Double_t mb = par[1] * tof_constants::amu;
    const Double_t mc = par[2] * tof_constants::amu;
    const Double_t md = par[3] * tof_constants::amu;
    const Double_t Ta = par[4] * par[5];
    const Double_t Tc = x[1] * par[6];

    const Double_t Ea = Ta + ma;
    const Double_t Ec = Tc + mc;
    const Double_t pa = std::sqrt(Ta * Ta + 2.0 * ma * Ta);
    const Double_t pc = std::sqrt(Tc * Tc + 2.0 * mc * Tc);

    return (Ea + mb) * Ec
           - pa * pc * std::cos(thetaC)
           - 0.5 * (ma * ma + mb * mb + mc * mc - md * md)
           - Ea * mb;
}


void TestTwoBody()
{
    gStyle->SetOptStat(111111111);
    gSystem->mkdir("output", kTRUE);

    // 10Be + p -> p + 10Be, ground-state elastic scattering.
    const Double_t beamEnergyPerU = 150.0;
    const Double_t excitationEnergyD = 0.0;
    const Double_t ma = tof_constants::m_10Be_AMU;
    const Double_t mb = tof_constants::m_1H_AMU;
    const Double_t mc = tof_constants::m_1H_AMU;
    const Double_t md = tof_constants::m_10Be_AMU;
    const Double_t Aa = tof_constants::A_10Be;
    const Double_t Ac = std::round(mc);

    const Double_t Ta = beamEnergyPerU * Aa;
    const Double_t massA = ma * tof_constants::amu;
    const Double_t massC = mc * tof_constants::amu;
    const Double_t paSquared = Ta * Ta + 2.0 * massA * Ta;
    const Double_t totalInitialEnergy = Ta + massA + massC;
    const Double_t maximumProtonEnergy =
        2.0 * massC * paSquared
        / (totalInitialEnergy * totalInitialEnergy - paSquared);
    const Double_t protonEnergyMax = 1.05 * maximumProtonEnergy / Ac;

    TF2* implicitCurve = new TF2(
        "implicitCurve",
        ImplicitKinematics,
        0.0,
        90.0,
        1.0e-3,
        protonEnergyMax,
        7);
    implicitCurve->SetParameters(
        ma, mb, mc, md, beamEnergyPerU, Aa, Ac);
    implicitCurve->SetNpx(1200);
    implicitCurve->SetNpy(1200);
    implicitCurve->SetLineColor(kRed + 1);
    implicitCurve->SetLineWidth(5);
    implicitCurve->SetTitle(
        "^{10}Be+p #rightarrow p+^{10}Be, T_{^{10}Be}=150 MeV/u;"
        "#theta_{p}^{lab} (deg);T_{p} (MeV/u)");

    Double_t contourLevel[1] = {0.0};
    implicitCurve->SetContour(1, contourLevel);

    TMultiGraph* referenceResult = TwoBodyKinematicsEnergyAngleProton(
        beamEnergyPerU,
        excitationEnergyD,
        ma,
        mb,
        mc,
        md,
        kBlue + 1,
        3,
        kDashed);

    TGraph* referencePhysicalBranch =
        (TGraph*)referenceResult->GetListOfGraphs()->First();

    TCanvas* canvas = new TCanvas(
        "canvasTestTwoBody",
        "Implicit contour and TwoBodyKinematics comparison",
        900,
        700);
    canvas->SetGrid();
    canvas->SetLeftMargin(0.13);
    canvas->SetBottomMargin(0.13);

    implicitCurve->Draw("CONT1");
    referencePhysicalBranch->Draw("L SAME");

    TLegend* legend = new TLegend(0.48, 0.75, 0.88, 0.88);
    legend->SetBorderSize(0);
    legend->AddEntry(implicitCurve, "Implicit function: F=0", "l");
    legend->AddEntry(
        referencePhysicalBranch,
        "TwoBodyKinematics.cc",
        "l");
    legend->Draw();

    canvas->SaveAs("output/TestTwoBody_10Be_p_150MeVu.pdf");
}
