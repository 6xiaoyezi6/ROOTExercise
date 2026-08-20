// ThreeBodyBoundaryT1Theta1.C
//
// 功能：读取三体运动学生成结果，分别绘制粒子1、粒子2和粒子B的
//       实验室系动能-角度分布，叠加解析三体允许区域边界和指定的两体弹性
//       散射曲线，输出 T1Theta1Correlation.pdf、T2Theta2Correlation.pdf 和
//       TBThetaBCorrelation.pdf。
//
// 方法：将 A+T->B+1+2 统一写成 A+T->i+(jk)；对每个 theta_i 在 T_i
//       方向使用余弦非均匀网格寻找边界方程的所有变号区间，再用二分法
//       精确求根；同时解析计算判别式为零的转折点，将该公共端点追加到
//       高、低能两支，使边界在最大角度处闭合。二维直方图先以 COLZ 绘制，
//       然后叠加线宽为 3 的 TGraph 边界和 TwoBodyKinematics 生成的
//       150 MeV/u 弹性散射曲线。
//
// 注意事项：输入必须包含 histograms/h1_TTheta、h2_TTheta 和 hB_TTheta；
//           输入直方图的动能轴是总动能 MeV，绘图前分别按质量数 1、4、6
//           换算为 MeV/u；质量输入为 u，内部能量为 MeV，角度为 degree。
//           粒子1角度显示范围为 0--90 degree，粒子2和粒子B为
//           0--50 degree；该范围只控制坐标显示和边界曲线的扫描上限，
//           不对输入 Monte Carlo 事件增加筛选。
//           T1 图叠加 alpha 弹性散射的反冲质子支，T2 图叠加出射 alpha 支，
//           TB 图叠加 6He 弹性散射的出射 6He 支。若解析转折点不在
//           当前角度或动能范围内，不向该图的边界分支追加端点。

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "TAxis.h"
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

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"


// ============================================================================
// 通用三体 Ti-theta_i 边界方程
//
// thetaDegree：deg
// kineticEnergyPerU：MeV/u
//
// par[0] = massA [u]
// par[1] = massT [u]
// par[2] = massI [u]
// par[3] = massJ [u]
// par[4] = massK [u]
// par[5] = beamEnergyPerU [MeV/u]
// par[6] = massNumberA
// par[7] = massNumberI
// ============================================================================

Double_t ThreeBodyBoundaryEquation(Double_t thetaDegree,
                                   Double_t kineticEnergyPerU,
                                   const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mI = par[2] * AmuToMeV;
    const Double_t mJ = par[3] * AmuToMeV;
    const Double_t mK = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t TI = par[7] * kineticEnergyPerU;

    if (TI < 0.0) return std::numeric_limits<Double_t>::quiet_NaN();

    const Double_t theta = thetaDegree * TMath::DegToRad();
    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t pI = std::sqrt(TI * (TI + 2.0 * mI));

    const Double_t leftSide =
        (mA + TA + mT) * (TI + mI) - pA * pI * std::cos(theta);
    const Double_t rightSide =
        0.5 * (mA * mA + mT * mT + mI * mI
               - (mJ + mK) * (mJ + mK))
        + mT * (mA + TA);

    return leftSide - rightSide;
}


// ============================================================================
// 根据能量守恒给出粒子 i 能够具有的最大动能，返回单位 MeV/u
// ============================================================================

Double_t GetMaximumPhysicalEnergyPerU(const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mI = par[2] * AmuToMeV;
    const Double_t mJ = par[3] * AmuToMeV;
    const Double_t mK = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t QValue = mA + mT - mI - mJ - mK;

    return (TA + QValue) / par[7];
}


// ============================================================================
// 解析计算三体边界高、低能支的公共转折点
//
// 将边界方程对粒子 i 的总能量 E_i 二次化，在判别式为零时两根合并。
// 返回的 thetaTurning 单位为 deg，energyTurningPerU 单位为 MeV/u。
// ============================================================================

Bool_t FindThreeBodyBoundaryTurningPoint(const Double_t* par,
                                         Double_t& thetaTurning,
                                         Double_t& energyTurningPerU)
{
    constexpr Double_t AmuToMeV = 931.49410242;
    constexpr Double_t BoundaryTolerance = 1e-12;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mI = par[2] * AmuToMeV;
    const Double_t mJ = par[3] * AmuToMeV;
    const Double_t mK = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t massNumberI = par[7];

    if (mI <= 0.0 || massNumberI <= 0.0) return kFALSE;

    const Double_t totalInitialEnergy = mA + TA + mT;
    const Double_t beamMomentumSquared = TA * (TA + 2.0 * mA);
    const Double_t boundaryConstant =
        0.5 * (mA * mA + mT * mT + mI * mI
               - (mJ + mK) * (mJ + mK))
        + mT * (mA + TA);

    if (beamMomentumSquared <= 0.0 || boundaryConstant <= 0.0) {
        return kFALSE;
    }

    Double_t cosThetaSquared =
        (totalInitialEnergy * totalInitialEnergy
         - boundaryConstant * boundaryConstant / (mI * mI))
        / beamMomentumSquared;

    if (!std::isfinite(cosThetaSquared)
        || cosThetaSquared < -BoundaryTolerance
        || cosThetaSquared > 1.0 + BoundaryTolerance) {
        return kFALSE;
    }

    cosThetaSquared =
        std::max(0.0, std::min(1.0, cosThetaSquared));

    const Double_t denominator =
        totalInitialEnergy * totalInitialEnergy
        - beamMomentumSquared * cosThetaSquared;
    if (denominator <= 0.0) return kFALSE;

    const Double_t totalEnergyTurning =
        totalInitialEnergy * boundaryConstant / denominator;
    const Double_t kineticEnergyTurning = totalEnergyTurning - mI;

    if (!std::isfinite(totalEnergyTurning)
        || kineticEnergyTurning < -BoundaryTolerance) {
        return kFALSE;
    }

    thetaTurning =
        std::acos(std::sqrt(cosThetaSquared)) * TMath::RadToDeg();
    energyTurningPerU =
        std::max(0.0, kineticEnergyTurning) / massNumberI;

    return std::isfinite(thetaTurning)
        && std::isfinite(energyTurningPerU);
}


// ============================================================================
// 已知 [energyLow,energyHigh] 内发生变号，使用二分法求边界方程的根
// ============================================================================

Double_t SolveThreeBodyBoundaryRoot(Double_t theta,
                                    Double_t energyLow,
                                    Double_t energyHigh,
                                    const Double_t* par)
{
    Double_t fLow = ThreeBodyBoundaryEquation(theta, energyLow, par);
    Double_t fHigh = ThreeBodyBoundaryEquation(theta, energyHigh, par);

    if (!std::isfinite(fLow) || !std::isfinite(fHigh)) {
        return std::numeric_limits<Double_t>::quiet_NaN();
    }
    if (std::abs(fLow) < 1e-8) return energyLow;
    if (std::abs(fHigh) < 1e-8) return energyHigh;
    if ((fLow > 0.0) == (fHigh > 0.0)) {
        return std::numeric_limits<Double_t>::quiet_NaN();
    }

    for (Int_t iteration = 0; iteration < 100; ++iteration) {
        const Double_t energyMiddle = 0.5 * (energyLow + energyHigh);
        const Double_t fMiddle =
            ThreeBodyBoundaryEquation(theta, energyMiddle, par);

        if (!std::isfinite(fMiddle)) {
            return std::numeric_limits<Double_t>::quiet_NaN();
        }
        if (std::abs(energyHigh - energyLow) < 1e-10) {
            return energyMiddle;
        }

        if ((fLow > 0.0) == (fMiddle > 0.0)) {
            energyLow = energyMiddle;
            fLow = fMiddle;
        } else {
            energyHigh = energyMiddle;
            fHigh = fMiddle;
        }
    }

    return 0.5 * (energyLow + energyHigh);
}


// ============================================================================
// 固定 theta_i，在 T_i 方向寻找所有边界根
// ============================================================================

std::vector<Double_t> FindThreeBodyBoundaryRootsAtTheta(
    Double_t theta,
    Double_t energyMin,
    Double_t energyMax,
    Int_t scanNEnergy,
    const Double_t* par)
{
    std::vector<Double_t> roots;

    const Double_t physicalMaximum = GetMaximumPhysicalEnergyPerU(par);
    const Double_t lowerEnergy = std::max(0.0, energyMin);
    const Double_t upperEnergy = std::min(energyMax, physicalMaximum);

    if (upperEnergy <= lowerEnergy) return roots;

    Double_t previousEnergy = lowerEnergy;
    Double_t previousF = ThreeBodyBoundaryEquation(theta, previousEnergy, par);

    if (std::isfinite(previousF) && std::abs(previousF) < 1e-8) {
        roots.push_back(previousEnergy);
    }

    for (Int_t i = 1; i <= scanNEnergy; ++i) {
        const Double_t t = static_cast<Double_t>(i) / scanNEnergy;
        const Double_t currentEnergy =
            lowerEnergy
            + 0.5 * (upperEnergy - lowerEnergy)
                  * (1.0 - std::cos(TMath::Pi() * t));
        const Double_t currentF =
            ThreeBodyBoundaryEquation(theta, currentEnergy, par);

        if (std::isfinite(previousF) && std::isfinite(currentF)) {
            if ((previousF > 0.0) != (currentF > 0.0)) {
                const Double_t root = SolveThreeBodyBoundaryRoot(
                    theta, previousEnergy, currentEnergy, par);
                if (std::isfinite(root)
                    && (roots.empty()
                        || std::abs(root - roots.back()) > 1e-6)) {
                    roots.push_back(root);
                }
            } else if (std::abs(currentF) < 1e-8) {
                if (roots.empty()
                    || std::abs(currentEnergy - roots.back()) > 1e-6) {
                    roots.push_back(currentEnergy);
                }
            }
        }

        previousEnergy = currentEnergy;
        previousF = currentF;
    }

    std::sort(roots.begin(), roots.end());
    return roots;
}


// ============================================================================
// 扫描 theta_i，分别建立低能支和高能支 TGraph
// ============================================================================

std::vector<TGraph*> CreateThreeBodyBoundaryGraphs(
    Double_t thetaMin,
    Double_t thetaMax,
    Double_t energyMin,
    Double_t energyMax,
    Int_t scanNTheta,
    Int_t scanNEnergy,
    const Double_t* par,
    const char* lowerGraphName,
    const char* upperGraphName)
{
    TGraph* lowerBranch = new TGraph();
    TGraph* upperBranch = new TGraph();

    lowerBranch->SetName(lowerGraphName);
    upperBranch->SetName(upperGraphName);

    lowerBranch->SetLineColor(kBlack);
    upperBranch->SetLineColor(kBlack);
    lowerBranch->SetLineStyle(kSolid);
    upperBranch->SetLineStyle(kSolid);
    lowerBranch->SetLineWidth(3);
    upperBranch->SetLineWidth(3);

    Int_t lowerPoint = 0;
    Int_t upperPoint = 0;

    for (Int_t i = 0; i <= scanNTheta; ++i) {
        const Double_t theta =
            thetaMin + (thetaMax - thetaMin) * i / scanNTheta;
        const std::vector<Double_t> roots =
            FindThreeBodyBoundaryRootsAtTheta(
                theta, energyMin, energyMax, scanNEnergy, par);

        if (roots.size() >= 1) {
            lowerBranch->SetPoint(lowerPoint++, theta, roots.front());
        }
        if (roots.size() >= 2) {
            upperBranch->SetPoint(upperPoint++, theta, roots.back());
        }
    }

    Double_t thetaTurning = 0.0;
    Double_t energyTurning = 0.0;
    const Bool_t hasTurningPoint = FindThreeBodyBoundaryTurningPoint(
        par, thetaTurning, energyTurning);
    const Double_t lowerEnergy = std::max(0.0, energyMin);
    const Double_t upperEnergy = std::min(
        energyMax, GetMaximumPhysicalEnergyPerU(par));
    constexpr Double_t EndpointTolerance = 1e-9;

    if (hasTurningPoint
        && lowerPoint > 0
        && upperPoint > 0
        && thetaTurning >= thetaMin - EndpointTolerance
        && thetaTurning <= thetaMax + EndpointTolerance
        && energyTurning >= lowerEnergy - EndpointTolerance
        && energyTurning <= upperEnergy + EndpointTolerance) {
        lowerBranch->SetPoint(lowerPoint++, thetaTurning, energyTurning);
        upperBranch->SetPoint(upperPoint++, thetaTurning, energyTurning);
    }

    return {lowerBranch, upperBranch};
}


void ScaleEnergyAxisToPerU(TAxis* axis, Double_t massNumber)
{
    axis->SetLimits(axis->GetXmin() / massNumber,
                    axis->GetXmax() / massNumber);
}


TGraph* GetFirstDrawableGraph(TMultiGraph* multiGraph)
{
    if (!multiGraph || !multiGraph->GetListOfGraphs()) return nullptr;

    TList* graphList = multiGraph->GetListOfGraphs();
    for (Int_t i = 0; i < graphList->GetSize(); ++i) {
        TGraph* graph = dynamic_cast<TGraph*>(graphList->At(i));
        if (graph && graph->GetN() > 1) return graph;
    }

    return nullptr;
}


void DrawGraphCollection(TMultiGraph* multiGraph)
{
    if (!multiGraph || !multiGraph->GetListOfGraphs()) return;

    TList* graphList = multiGraph->GetListOfGraphs();
    for (Int_t i = 0; i < graphList->GetSize(); ++i) {
        TGraph* graph = dynamic_cast<TGraph*>(graphList->At(i));
        if (graph && graph->GetN() > 1) graph->Draw("L SAME");
    }
}


void DrawEnergyAngleCorrelation(
    TCanvas* canvas,
    TH2D* histogram,
    const std::vector<TGraph*>& boundaryGraphs,
    TMultiGraph* elasticGraphs,
    const char* yAxisTitle,
    const char* boundaryLegendLabel,
    const char* elasticLegendLabel,
    const char* outputFileName)
{
    canvas->Clear();

    histogram->SetTitle("");
    histogram->SetStats(kFALSE);
    histogram->GetYaxis()->SetTitle(yAxisTitle);
    histogram->Draw("COLZ");

    TGraph* boundaryLegendGraph = nullptr;
    for (std::size_t i = 0; i < boundaryGraphs.size(); ++i) {
        TGraph* graph = boundaryGraphs[i];
        if (graph && graph->GetN() > 1) {
            graph->Draw("L SAME");
            if (!boundaryLegendGraph) boundaryLegendGraph = graph;
        }
    }

    DrawGraphCollection(elasticGraphs);
    TGraph* elasticLegendGraph = GetFirstDrawableGraph(elasticGraphs);

    TLegend* legend = new TLegend(0.15, 0.73, 0.63, 0.87);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    if (boundaryLegendGraph) {
        legend->AddEntry(boundaryLegendGraph, boundaryLegendLabel, "l");
    }
    if (elasticLegendGraph) {
        legend->AddEntry(elasticLegendGraph, elasticLegendLabel, "l");
    }

    legend->Draw();
    canvas->SaveAs(outputFileName);
}


// ============================================================================
// 主函数
// ============================================================================

void ThreeBodyBoundaryT1Theta1()
{
    gStyle->SetOptStat(111111111);
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
    const Double_t massNumberB = 6.0;
    const Double_t massNumber1 = 1.0;
    const Double_t massNumber2 = 4.0;

    // ------------------------------------------------------------------------
    // 绘图范围与求根扫描精度
    // ------------------------------------------------------------------------

    const Double_t thetaDisplayMin = 0.0;
    const Double_t thetaDisplayMax1 = 90.0;
    const Double_t thetaDisplayMax2 = 40.0;
    const Double_t thetaDisplayMaxB = 40.0;
    const Double_t energyDisplayMin = 0.0;
    const Double_t energyDisplayMax = 800.0;
    const Int_t thetaScanN = 5000;
    const Int_t energyScanN = 3000;

    // ------------------------------------------------------------------------
    // 输入 Monte Carlo
    // ------------------------------------------------------------------------

    TFile* inputFile =
        TFile::Open("output/PureThreeBodyKinematics.root", "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cout
            << "[ERROR] Cannot open output/PureThreeBodyKinematics.root"
            << std::endl;
        return;
    }

    TDirectory* histogramDirectory =
        (TDirectory*)inputFile->Get("histograms");
    if (!histogramDirectory) {
        std::cout << "[ERROR] Cannot find histograms directory" << std::endl;
        inputFile->Close();
        return;
    }

    TH2D* h1TTheta = (TH2D*)histogramDirectory->Get("h1_TTheta");
    TH2D* h2TTheta = (TH2D*)histogramDirectory->Get("h2_TTheta");
    TH2D* hBTTheta = (TH2D*)histogramDirectory->Get("hB_TTheta");

    if (!h1TTheta || !h2TTheta || !hBTTheta) {
        std::cout
            << "[ERROR] Cannot find h1_TTheta, h2_TTheta, or hB_TTheta"
            << std::endl;
        inputFile->Close();
        return;
    }

    ScaleEnergyAxisToPerU(h1TTheta->GetYaxis(), massNumber1);
    ScaleEnergyAxisToPerU(h2TTheta->GetYaxis(), massNumber2);
    ScaleEnergyAxisToPerU(hBTTheta->GetYaxis(), massNumberB);

    TH2D* histograms[3] = {h1TTheta, h2TTheta, hBTTheta};
    const Double_t thetaDisplayMax[3] = {
        thetaDisplayMax1, thetaDisplayMax2, thetaDisplayMaxB
    };
    for (Int_t i = 0; i < 3; ++i) {
        histograms[i]->GetXaxis()->SetRangeUser(
            thetaDisplayMin, thetaDisplayMax[i]);
        histograms[i]->GetYaxis()->SetRangeUser(
            energyDisplayMin, energyDisplayMax);
    }

    // ------------------------------------------------------------------------
    // i=1、2、B 时的三体边界参数
    // ------------------------------------------------------------------------

    const Double_t parameters1[8] = {
        massA, massT, mass1, massB, mass2,
        beamEnergyPerU, massNumberA, massNumber1
    };
    const Double_t parameters2[8] = {
        massA, massT, mass2, massB, mass1,
        beamEnergyPerU, massNumberA, massNumber2
    };
    const Double_t parametersB[8] = {
        massA, massT, massB, mass1, mass2,
        beamEnergyPerU, massNumberA, massNumberB
    };

    std::vector<TGraph*> boundaryGraphs1 = CreateThreeBodyBoundaryGraphs(
        thetaDisplayMin,
        thetaDisplayMax1,
        h1TTheta->GetYaxis()->GetXmin(),
        h1TTheta->GetYaxis()->GetXmax(),
        thetaScanN,
        energyScanN,
        parameters1,
        "threeBodyBoundary1Lower",
        "threeBodyBoundary1Upper");

    std::vector<TGraph*> boundaryGraphs2 = CreateThreeBodyBoundaryGraphs(
        thetaDisplayMin,
        thetaDisplayMax2,
        h2TTheta->GetYaxis()->GetXmin(),
        h2TTheta->GetYaxis()->GetXmax(),
        thetaScanN,
        energyScanN,
        parameters2,
        "threeBodyBoundary2Lower",
        "threeBodyBoundary2Upper");

    std::vector<TGraph*> boundaryGraphsB = CreateThreeBodyBoundaryGraphs(
        thetaDisplayMin,
        thetaDisplayMaxB,
        hBTTheta->GetYaxis()->GetXmin(),
        hBTTheta->GetYaxis()->GetXmax(),
        thetaScanN,
        energyScanN,
        parametersB,
        "threeBodyBoundaryBLower",
        "threeBodyBoundaryBUpper");

    // ------------------------------------------------------------------------
    // 150 MeV/u 两体弹性散射参考曲线
    // ------------------------------------------------------------------------

    TMultiGraph* alphaElasticProton = TwoBodyKinematicsEnergyAngleProton(
        beamEnergyPerU,
        0.0,
        mass2,
        massT,
        mass1,
        mass2,
        kGreen + 2,
        2,
        kSolid,
        1.0);

    TMultiGraph* alphaElasticResidue = TwoBodyKinematicsEnergyAngleResidue(
        beamEnergyPerU,
        0.0,
        mass2,
        massT,
        mass1,
        mass2,
        kGreen + 2,
        2,
        kSolid,
        1.0);

    TMultiGraph* helium6ElasticResidue = TwoBodyKinematicsEnergyAngleResidue(
        beamEnergyPerU,
        0.0,
        massB,
        massT,
        mass1,
        massB,
        kBlue + 1,
        2,
        kSolid,
        1.0);

    // ------------------------------------------------------------------------
    // 绘图并输出三个 PDF
    // ------------------------------------------------------------------------

    TCanvas* canvas = new TCanvas(
        "canvasThreeBodyBoundary",
        "Three-body kinematic boundaries",
        900,
        700);

    DrawEnergyAngleCorrelation(
        canvas,
        h1TTheta,
        boundaryGraphs1,
        alphaElasticProton,
        "T_{1}^{lab} [MeV/u]",
        "Three-body boundary: M_{B2}=m_{B}+m_{2}",
        "^{4}He(p,p)^{4}He: recoil p, 150 MeV/u",
        "output/T1Theta1Correlation.pdf");

    DrawEnergyAngleCorrelation(
        canvas,
        h2TTheta,
        boundaryGraphs2,
        alphaElasticResidue,
        "T_{2}^{lab} [MeV/u]",
        "Three-body boundary: M_{B1}=m_{B}+m_{1}",
        "^{4}He(p,p)^{4}He: outgoing ^{4}He, 150 MeV/u",
        "output/T2Theta2Correlation.pdf");

    DrawEnergyAngleCorrelation(
        canvas,
        hBTTheta,
        boundaryGraphsB,
        helium6ElasticResidue,
        "T_{B}^{lab} [MeV/u]",
        "Three-body boundary: M_{12}=m_{1}+m_{2}",
        "^{6}He(p,p)^{6}He: outgoing ^{6}He, 150 MeV/u",
        "output/TBThetaBCorrelation.pdf");

    inputFile->Close();
}
