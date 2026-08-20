// ThreeBodyBoundaryT1Theta1.C
//
// 功能：读取三体运动学生成结果，绘制质子 T1-角度分布，并叠加解析三体边界
//       及 alpha+p 弹性散射参考曲线，输出 boundary PDF。
//
// 方法：对 theta1 进行一维扫描；对于每一个固定 theta1，在 T1 方向寻找
//       F(theta1,T1)=0 的所有根，并使用二分法精确求根，最后以 TGraph 绘制。
//       T1 扫描采用余弦非均匀网格，在 T1=0 和最大允许动能附近自动加密。
//
// 三体边界：
//       M_{B2} = m_B + m_2
//
//       F(theta1,T1)
//       = (m_A+T_A+m_T)(T_1+m_1)
//         - p_A p_1 cos(theta_1)
//         - 1/2[m_A^2+m_T^2+m_1^2-(m_B+m_2)^2]
//         - m_T(m_A+T_A)
//       = 0
//
// 单位：
//       输入质量：u (AMU)
//       束流动能：MeV/u
//       图中 T1：MeV/u
//       程序内部质量：MeV/c^2
//       程序内部动能：总动能 MeV
//
// 注意事项：输入必须包含 histograms/h1_TTheta；绘图时固定
//           theta1 为横轴、T1 为纵轴，并清空顶部标题、关闭统计框。

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TStyle.h"
#include "TSystem.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/constants.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"


// ============================================================================
// 三体 T1-theta1 边界方程
//
// theta1：deg
// T1PerU：MeV/u
//
// par[0] = massA [u]
// par[1] = massT [u]
// par[2] = massB [u]
// par[3] = mass1 [u]
// par[4] = mass2 [u]
// par[5] = beamEnergyPerU [MeV/u]
// par[6] = massNumberA
// par[7] = massNumber1
// ============================================================================

Double_t ThreeBodyBoundaryEquationT1Theta1(Double_t theta1Degree, Double_t T1PerU, const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t T1 = par[7] * T1PerU;

    if (T1 < 0.0) return std::numeric_limits<Double_t>::quiet_NaN();

    const Double_t theta1 = theta1Degree * TMath::DegToRad();
    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t p1 = std::sqrt(T1 * (T1 + 2.0 * m1));

    const Double_t leftSide = (mA + TA + mT) * (T1 + m1) - pA * p1 * std::cos(theta1);
    const Double_t rightSide = 0.5 * (mA * mA + mT * mT + m1 * m1 - (mB + m2) * (mB + m2)) + mT * (mA + TA);

    return leftSide - rightSide;
}


// ============================================================================
// 根据能量守恒给出粒子1能够具有的最大总动能
//
// T1 <= TA + Q
// Q = mA + mT - mB - m1 - m2
//
// 返回值单位：MeV/u
// ============================================================================

Double_t GetMaximumPhysicalT1PerU(const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t QValue = mA + mT - mB - m1 - m2;

    return (TA + QValue) / par[7];
}


// ============================================================================
// 已知 [TLow,THigh] 内发生变号，使用二分法求 F(theta1,T1)=0
// ============================================================================

Double_t SolveThreeBodyBoundaryRootT1Theta1(Double_t theta1, Double_t TLow, Double_t THigh, const Double_t* par)
{
    Double_t fLow = ThreeBodyBoundaryEquationT1Theta1(theta1, TLow, par);
    Double_t fHigh = ThreeBodyBoundaryEquationT1Theta1(theta1, THigh, par);

    if (!std::isfinite(fLow) || !std::isfinite(fHigh)) return std::numeric_limits<Double_t>::quiet_NaN();
    if (std::abs(fLow) < 1e-8) return TLow;
    if (std::abs(fHigh) < 1e-8) return THigh;
    if ((fLow > 0.0) == (fHigh > 0.0)) return std::numeric_limits<Double_t>::quiet_NaN();

    for (Int_t iteration = 0; iteration < 100; ++iteration) {
        const Double_t TMiddle = 0.5 * (TLow + THigh);
        const Double_t fMiddle = ThreeBodyBoundaryEquationT1Theta1(theta1, TMiddle, par);

        if (!std::isfinite(fMiddle)) return std::numeric_limits<Double_t>::quiet_NaN();
        if (std::abs(THigh - TLow) < 1e-10) return TMiddle;

        if ((fLow > 0.0) == (fMiddle > 0.0)) {
            TLow = TMiddle;
            fLow = fMiddle;
        } else {
            THigh = TMiddle;
            fHigh = fMiddle;
        }
    }

    return 0.5 * (TLow + THigh);
}


// ============================================================================
// 固定 theta1 后，在 T1 方向寻找所有 F(theta1,T1)=0 的根
//
// T1 使用余弦网格：
//       T1 = Tmin + (Tmax-Tmin)/2 * [1-cos(pi*t)]
//
// 因此在 Tmin 和 Tmax 两端自动加密。
// 一个 theta1 下通常有两个根：低能支和高能支。
// ============================================================================

std::vector<Double_t> FindThreeBodyBoundaryRootsAtTheta1(Double_t theta1, Double_t energyMin, Double_t energyMax, Int_t scanNEnergy, const Double_t* par)
{
    std::vector<Double_t> roots;

    const Double_t physicalMaximum = GetMaximumPhysicalT1PerU(par);
    const Double_t lowerEnergy = std::max(0.0, energyMin);
    const Double_t upperEnergy = std::min(energyMax, physicalMaximum);

    if (upperEnergy <= lowerEnergy) return roots;

    Double_t previousEnergy = lowerEnergy;
    Double_t previousF = ThreeBodyBoundaryEquationT1Theta1(theta1, previousEnergy, par);

    if (std::isfinite(previousF) && std::abs(previousF) < 1e-8) roots.push_back(previousEnergy);

    for (Int_t i = 1; i <= scanNEnergy; ++i) {
        const Double_t t = static_cast<Double_t>(i) / scanNEnergy;
        const Double_t currentEnergy = lowerEnergy + 0.5 * (upperEnergy - lowerEnergy) * (1.0 - std::cos(TMath::Pi() * t));
        const Double_t currentF = ThreeBodyBoundaryEquationT1Theta1(theta1, currentEnergy, par);

        if (std::isfinite(previousF) && std::isfinite(currentF)) {
            if ((previousF > 0.0) != (currentF > 0.0)) {
                const Double_t root = SolveThreeBodyBoundaryRootT1Theta1(theta1, previousEnergy, currentEnergy, par);
                if (std::isfinite(root) && (roots.empty() || std::abs(root - roots.back()) > 1e-6)) roots.push_back(root);
            } else if (std::abs(currentF) < 1e-8) {
                if (roots.empty() || std::abs(currentEnergy - roots.back()) > 1e-6) roots.push_back(currentEnergy);
            }
        }

        previousEnergy = currentEnergy;
        previousF = currentF;
    }

    std::sort(roots.begin(), roots.end());
    return roots;
}


// ============================================================================
// 扫描 theta1，分别建立低能支和高能支 TGraph
//
// 返回：
//       graphs[0] = lower T1 branch
//       graphs[1] = upper T1 branch
// ============================================================================

std::vector<TGraph*> CreateThreeBodyBoundaryGraphsT1Theta1(Double_t thetaMin, Double_t thetaMax, Double_t energyMin, Double_t energyMax, Int_t scanNTheta, Int_t scanNEnergy, const Double_t* par)
{
    TGraph* lowerBranch = new TGraph();
    TGraph* upperBranch = new TGraph();

    lowerBranch->SetName("threeBodyBoundaryLower");
    upperBranch->SetName("threeBodyBoundaryUpper");

    lowerBranch->SetLineColor(kBlack);
    upperBranch->SetLineColor(kBlack);
    lowerBranch->SetLineStyle(kSolid);
    upperBranch->SetLineStyle(kSolid);
    lowerBranch->SetLineWidth(2);
    upperBranch->SetLineWidth(2);

    Int_t lowerPoint = 0;
    Int_t upperPoint = 0;

    for (Int_t i = 0; i <= scanNTheta; ++i) {
        const Double_t theta1 = thetaMin + (thetaMax - thetaMin) * i / scanNTheta;
        const std::vector<Double_t> roots = FindThreeBodyBoundaryRootsAtTheta1(theta1, energyMin, energyMax, scanNEnergy, par);

        if (roots.size() >= 1) lowerBranch->SetPoint(lowerPoint++, theta1, roots.front());
        if (roots.size() >= 2) upperBranch->SetPoint(upperPoint++, theta1, roots.back());
    }

    return {lowerBranch, upperBranch};
}


// ============================================================================
// 主函数
// ============================================================================

void ThreeBodyBoundaryT1Theta1()
{
    gStyle->SetOptStat(0);
    gSystem->mkdir("output", kTRUE);

    // ------------------------------------------------------------------------
    // 用户输入：10Be + p -> 6He + p + alpha
    // ------------------------------------------------------------------------

    const Double_t massA = 10.0135338;       // u
    const Double_t massT = 1.007276466621;   // u
    const Double_t massB = 6.01888589;       // u
    const Double_t mass1 = 1.007276466621;   // u
    const Double_t mass2 = 4.001506179127;   // u

    const Double_t beamEnergyPerU = 150.0;   // MeV/u
    const Double_t massNumberA = 10.0;
    const Double_t massNumber1 = 1.0;

    // ------------------------------------------------------------------------
    // 绘图范围
    // ------------------------------------------------------------------------

    const Double_t thetaMin = 0.0;
    const Double_t thetaMax = 90.0;
    const Double_t energyDisplayMin = 0.0;
    const Double_t energyDisplayMax = 800.0;

    // ------------------------------------------------------------------------
    // 一维求根扫描精度
    //
    // thetaScanN：theta1 方向采样点数
    // energyScanN：每个 theta1 下 T1 方向扫描点数
    //
    // T1 使用余弦网格，因此即使低能根只有 ~0.06 MeV 也能够稳定找到。
    // ------------------------------------------------------------------------

    const Int_t thetaScanN = 5000;
    const Int_t energyScanN = 3000;

    // ------------------------------------------------------------------------
    // 输入 Monte Carlo
    // ------------------------------------------------------------------------

    TFile* inputFile = TFile::Open("output/PureThreeBodyKinematics.root", "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cout << "[ERROR] Cannot open output/PureThreeBodyKinematics.root" << std::endl;
        return;
    }

    TDirectory* histogramDirectory = (TDirectory*)inputFile->Get("histograms");
    if (!histogramDirectory) return;

    TH2D* h1TTheta = (TH2D*)histogramDirectory->Get("h1_TTheta");

    if (!h1TTheta) return;

    const Double_t energyMin = h1TTheta->GetYaxis()->GetXmin();
    const Double_t energyMax = h1TTheta->GetYaxis()->GetXmax();

    h1TTheta->GetXaxis()->SetRangeUser(thetaMin, thetaMax);
    h1TTheta->GetYaxis()->SetRangeUser(energyDisplayMin, energyDisplayMax);
    h1TTheta->SetTitle("");
    h1TTheta->GetYaxis()->SetTitle("T_{1}^{lab} [MeV/u]");

    // ------------------------------------------------------------------------
    // 三体边界参数
    // ------------------------------------------------------------------------

    const Double_t reactionParameters[8] = {
        massA,
        massT,
        massB,
        mass1,
        mass2,
        beamEnergyPerU,
        massNumberA,
        massNumber1
    };

    // ------------------------------------------------------------------------
    // 一维扫描 + 二分法生成三体边界
    // ------------------------------------------------------------------------

    std::vector<TGraph*> boundaryGraphs = CreateThreeBodyBoundaryGraphsT1Theta1(thetaMin, thetaMax, energyMin, energyMax, thetaScanN, energyScanN, reactionParameters);

    // ------------------------------------------------------------------------
    // alpha + p 两体弹性散射参考曲线
    // ------------------------------------------------------------------------

    TMultiGraph* alphaProtonElastic = TwoBodyKinematicsEnergyAngleProton(
        150.0,
        0.0,
        tof_constants::m_4He_AMU,
        tof_constants::m_1H_AMU,
        tof_constants::m_1H_AMU,
        tof_constants::m_4He_AMU,
        kGreen + 2,
        2,
        kSolid,1);

    TGraph* alphaProtonPhysicalBranch = (TGraph*)alphaProtonElastic->GetListOfGraphs()->First();

    // ------------------------------------------------------------------------
    // 绘图
    // ------------------------------------------------------------------------

    TCanvas* canvas = new TCanvas("canvasThreeBodyBoundary", "Three-body kinematic boundary", 900, 700);

    h1TTheta->Draw("COLZ");

    for (TGraph* graph : boundaryGraphs) {
        if (graph && graph->GetN() > 1) graph->Draw("L SAME");
    }

    if (alphaProtonPhysicalBranch) alphaProtonPhysicalBranch->Draw("L SAME");

    // ------------------------------------------------------------------------
    // 图例
    // ------------------------------------------------------------------------

    TLegend* legend = new TLegend(0.16, 0.73, 0.54, 0.87);
    legend->SetBorderSize(0);

    TGraph* boundaryLegendGraph = nullptr;
    for (TGraph* graph : boundaryGraphs) {
        if (graph && graph->GetN() > 1) {
            boundaryLegendGraph = graph;
            break;
        }
    }

    if (boundaryLegendGraph) legend->AddEntry(boundaryLegendGraph, "Implicit boundary: M_{B2}=m_{B}+m_{2}", "l");
    if (alphaProtonPhysicalBranch) legend->AddEntry(alphaProtonPhysicalBranch, "^{4}He+p elastic, T_{#alpha}=150 MeV/u", "l");

    legend->Draw();

    canvas->SaveAs("output/T1Theta1Correlation.pdf");

    inputFile->Close();
}
