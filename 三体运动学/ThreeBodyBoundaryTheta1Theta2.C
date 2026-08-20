// ThreeBodyBoundaryTheta1Theta2.C
//
// 功能：读取 output/PureThreeBodyKinematics.root 中的 theta1-theta2 二维分布，
//       用三体四动量守恒隐函数 F12=0 扫描角度允许区域的闭合边界，
//       并叠加 150 MeV/u 4He(p,p)4He 的质子-alpha 角度关联，输出
//       output/Theta1Theta2.pdf；图中不显示统计框，图例放在右上方。
//
// 方法：对 T1、T2（MeV/u）使用余弦网格扫描，取 DeltaPhi12=0 和 pi；
//       对每个 theta1 将 F12 写成 K+A*cos(theta2)+B*sin(theta2)=0，
//       解析求出 [0,180 deg] 内的全部 theta2 根。对每个 theta1 保留最小和
//       最大 theta2，先沿左边界正向、再沿右边界反向写入 TGraph，最后
//       显式追加首点，保证黑色线宽3的边界闭合。坐标为 X=theta2、Y=theta1。
//
// 注意事项：输入直方图 hTheta1_Theta2 必须使用 X=theta2、Y=theta1；
//           质量单位为 u，内部总动能为 MeV，扫描动能为 MeV/u，角度为 degree。
//           黑色曲线是对有限 T1/T2/theta1 网格上隐函数根的投影包络，
//           精度由 theta1ScanN、T1ScanN 和 T2ScanN 控制；不使用蒙卡分箱
//           决定边界，但彩色背景密度仍受蒙卡统计和分箱影响。

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

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"


// ============================================================================
// 三体 theta1-theta2 隐函数
//
// par[0] = massA [u]
// par[1] = massT [u]
// par[2] = massB [u]
// par[3] = mass1 [u]
// par[4] = mass2 [u]
// par[5] = beamEnergyPerU [MeV/u]
// par[6] = massNumberA
// par[7] = massNumber1
// par[8] = massNumber2
// ============================================================================

Double_t ThreeBodyTheta1Theta2ImplicitEquation(
    Double_t theta1Degree,
    Double_t theta2Degree,
    Double_t T1PerU,
    Double_t T2PerU,
    Double_t cosDeltaPhi12,
    const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t T1 = par[7] * T1PerU;
    const Double_t T2 = par[8] * T2PerU;

    if (T1 < 0.0 || T2 < 0.0) {
        return std::numeric_limits<Double_t>::quiet_NaN();
    }

    const Double_t EA = mA + TA;
    const Double_t E1 = m1 + T1;
    const Double_t E2 = m2 + T2;
    const Double_t EB = EA + mT - E1 - E2;
    if (EB < mB) return std::numeric_limits<Double_t>::quiet_NaN();

    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t p1 = std::sqrt(T1 * (T1 + 2.0 * m1));
    const Double_t p2 = std::sqrt(T2 * (T2 + 2.0 * m2));
    const Double_t s = mA * mA + mT * mT + 2.0 * mT * EA;

    const Double_t theta1 = theta1Degree * TMath::DegToRad();
    const Double_t theta2 = theta2Degree * TMath::DegToRad();
    const Double_t cosTheta12 =
        std::cos(theta1) * std::cos(theta2)
        + std::sin(theta1) * std::sin(theta2) * cosDeltaPhi12;

    return s + m1 * m1 + m2 * m2 - mB * mB
        - 2.0 * (EA + mT) * (E1 + E2)
        + 2.0 * pA
              * (p1 * std::cos(theta1) + p2 * std::cos(theta2))
        + 2.0 * E1 * E2
        - 2.0 * p1 * p2 * cosTheta12;
}


void UpdateTheta2EnvelopeWithRoot(Double_t theta2Degree,
                                  Double_t theta2Min,
                                  Double_t theta2Max,
                                  Bool_t& hasRoot,
                                  Double_t& lowerTheta2,
                                  Double_t& upperTheta2)
{
    constexpr Double_t AngleTolerance = 1e-10;

    if (!std::isfinite(theta2Degree)
        || theta2Degree < theta2Min - AngleTolerance
        || theta2Degree > theta2Max + AngleTolerance) {
        return;
    }

    const Double_t boundedTheta2 =
        std::max(theta2Min, std::min(theta2Max, theta2Degree));

    if (!hasRoot) {
        lowerTheta2 = boundedTheta2;
        upperTheta2 = boundedTheta2;
        hasRoot = kTRUE;
        return;
    }

    lowerTheta2 = std::min(lowerTheta2, boundedTheta2);
    upperTheta2 = std::max(upperTheta2, boundedTheta2);
}


// ============================================================================
// 固定 theta1、T1、T2 和 cos(DeltaPhi12)，解析求解 theta2
//
// F12 = K + A*cos(theta2) + B*sin(theta2) = 0
//     = K + R*cos(theta2-delta) = 0
// ============================================================================

void FindTheta2RootsAndUpdateEnvelope(
    Double_t theta1Degree,
    Double_t T1PerU,
    Double_t T2PerU,
    Double_t cosDeltaPhi12,
    Double_t theta2Min,
    Double_t theta2Max,
    const Double_t* par,
    Bool_t& hasRoot,
    Double_t& lowerTheta2,
    Double_t& upperTheta2)
{
    constexpr Double_t AmuToMeV = 931.49410242;
    constexpr Double_t EquationTolerance = 1e-12;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t T1 = par[7] * T1PerU;
    const Double_t T2 = par[8] * T2PerU;

    if (T1 <= 0.0 || T2 <= 0.0) return;

    const Double_t EA = mA + TA;
    const Double_t E1 = m1 + T1;
    const Double_t E2 = m2 + T2;
    const Double_t EB = EA + mT - E1 - E2;
    if (EB < mB) return;

    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t p1 = std::sqrt(T1 * (T1 + 2.0 * m1));
    const Double_t p2 = std::sqrt(T2 * (T2 + 2.0 * m2));
    const Double_t s = mA * mA + mT * mT + 2.0 * mT * EA;
    const Double_t theta1 = theta1Degree * TMath::DegToRad();
    const Double_t cosTheta1 = std::cos(theta1);
    const Double_t sinTheta1 = std::sin(theta1);

    const Double_t constantTerm =
        s + m1 * m1 + m2 * m2 - mB * mB
        - 2.0 * (EA + mT) * (E1 + E2)
        + 2.0 * pA * p1 * cosTheta1
        + 2.0 * E1 * E2;
    const Double_t cosineCoefficient =
        2.0 * p2 * (pA - p1 * cosTheta1);
    const Double_t sineCoefficient =
        -2.0 * p1 * p2 * sinTheta1 * cosDeltaPhi12;
    const Double_t amplitude =
        std::sqrt(cosineCoefficient * cosineCoefficient
                  + sineCoefficient * sineCoefficient);

    if (amplitude <= 0.0) return;

    Double_t normalizedRoot = -constantTerm / amplitude;
    if (!std::isfinite(normalizedRoot)
        || normalizedRoot < -1.0 - EquationTolerance
        || normalizedRoot > 1.0 + EquationTolerance) {
        return;
    }

    normalizedRoot =
        std::max(-1.0, std::min(1.0, normalizedRoot));

    const Double_t phase =
        std::atan2(sineCoefficient, cosineCoefficient);
    const Double_t rootOffset = std::acos(normalizedRoot);

    for (Int_t signIndex = 0; signIndex < 2; ++signIndex) {
        Double_t sign = -1.0;
        if (signIndex == 1) sign = 1.0;
        const Double_t baseRoot = phase + sign * rootOffset;

        for (Int_t periodIndex = -1; periodIndex <= 1; ++periodIndex) {
            const Double_t theta2 =
                baseRoot + 2.0 * TMath::Pi() * periodIndex;
            const Double_t theta2Degree =
                theta2 * TMath::RadToDeg();

            if (std::abs(ThreeBodyTheta1Theta2ImplicitEquation(
                    theta1Degree,
                    theta2Degree,
                    T1PerU,
                    T2PerU,
                    cosDeltaPhi12,
                    par)) > 1e-4) {
                continue;
            }

            UpdateTheta2EnvelopeWithRoot(
                theta2Degree,
                theta2Min,
                theta2Max,
                hasRoot,
                lowerTheta2,
                upperTheta2);
        }
    }
}


// ============================================================================
// 扫描 T1、T2、DeltaPhi12 和 theta1，构造闭合角度边界
// ============================================================================

TGraph* CreateTheta1Theta2BoundaryGraph(
    Double_t theta1Min,
    Double_t theta1Max,
    Double_t theta2Min,
    Double_t theta2Max,
    Int_t theta1ScanN,
    Int_t T1ScanN,
    Int_t T2ScanN,
    const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;
    const Double_t TA = par[5] * par[6];
    const Double_t QValue = mA + mT - mB - m1 - m2;
    const Double_t availableKineticEnergy = TA + QValue;
    const Double_t maximumT1PerU =
        availableKineticEnergy / par[7];

    std::vector<Double_t> theta1Values;
    std::vector<Double_t> lowerTheta2Values;
    std::vector<Double_t> upperTheta2Values;

    for (Int_t thetaIndex = 0; thetaIndex <= theta1ScanN; ++thetaIndex) {
        const Double_t theta1 =
            theta1Min
            + (theta1Max - theta1Min) * thetaIndex / theta1ScanN;
        Bool_t hasRoot = kFALSE;
        Double_t lowerTheta2 = 0.0;
        Double_t upperTheta2 = 0.0;

        for (Int_t T1Index = 0; T1Index < T1ScanN; ++T1Index) {
            const Double_t T1Fraction =
                (T1Index + 0.5) / static_cast<Double_t>(T1ScanN);
            const Double_t T1PerU =
                0.5 * maximumT1PerU
                * (1.0 - std::cos(TMath::Pi() * T1Fraction));
            const Double_t remainingKineticEnergy =
                availableKineticEnergy - par[7] * T1PerU;

            if (remainingKineticEnergy <= 0.0) continue;

            const Double_t maximumT2PerU =
                remainingKineticEnergy / par[8];

            for (Int_t T2Index = 0; T2Index < T2ScanN; ++T2Index) {
                const Double_t T2Fraction =
                    (T2Index + 0.5) / static_cast<Double_t>(T2ScanN);
                const Double_t T2PerU =
                    0.5 * maximumT2PerU
                    * (1.0 - std::cos(TMath::Pi() * T2Fraction));

                FindTheta2RootsAndUpdateEnvelope(
                    theta1,
                    T1PerU,
                    T2PerU,
                    1.0,
                    theta2Min,
                    theta2Max,
                    par,
                    hasRoot,
                    lowerTheta2,
                    upperTheta2);

                FindTheta2RootsAndUpdateEnvelope(
                    theta1,
                    T1PerU,
                    T2PerU,
                    -1.0,
                    theta2Min,
                    theta2Max,
                    par,
                    hasRoot,
                    lowerTheta2,
                    upperTheta2);
            }
        }

        if (hasRoot) {
            theta1Values.push_back(theta1);
            lowerTheta2Values.push_back(lowerTheta2);
            upperTheta2Values.push_back(upperTheta2);
        }
    }

    TGraph* boundary = new TGraph();
    boundary->SetName("threeBodyTheta1Theta2Boundary");
    boundary->SetLineColor(kBlack);
    boundary->SetLineWidth(3);
    boundary->SetLineStyle(kSolid);

    if (theta1Values.empty()) return boundary;

    Int_t pointIndex = 0;
    for (std::size_t i = 0; i < theta1Values.size(); ++i) {
        boundary->SetPoint(
            pointIndex++, lowerTheta2Values[i], theta1Values[i]);
    }

    for (Int_t i = static_cast<Int_t>(theta1Values.size()) - 1;
         i >= 0;
         --i) {
        boundary->SetPoint(
            pointIndex++, upperTheta2Values[i], theta1Values[i]);
    }

    boundary->SetPoint(
        pointIndex++, lowerTheta2Values.front(), theta1Values.front());

    return boundary;
}


void DrawAngleAngleGraphCollection(TMultiGraph* multiGraph)
{
    if (!multiGraph || !multiGraph->GetListOfGraphs()) return;

    TList* graphList = multiGraph->GetListOfGraphs();
    for (Int_t i = 0; i < graphList->GetSize(); ++i) {
        TGraph* graph = dynamic_cast<TGraph*>(graphList->At(i));
        if (graph && graph->GetN() > 1) graph->Draw("L SAME");
    }
}


TGraph* GetFirstAngleAngleGraph(TMultiGraph* multiGraph)
{
    if (!multiGraph || !multiGraph->GetListOfGraphs()) return nullptr;

    TList* graphList = multiGraph->GetListOfGraphs();
    for (Int_t i = 0; i < graphList->GetSize(); ++i) {
        TGraph* graph = dynamic_cast<TGraph*>(graphList->At(i));
        if (graph && graph->GetN() > 1) return graph;
    }

    return nullptr;
}


// ============================================================================
// 主函数
// ============================================================================

void ThreeBodyBoundaryTheta1Theta2()
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
    const Double_t massNumber2 = 4.0;

    const Double_t theta1Min = 0.0;
    const Double_t theta1Max = 90.0;
    const Double_t theta2Min = 0.0;
    const Double_t theta2Max = 40.0;

    // theta1 步长为 0.18 deg；T1/T2 使用端点加密的余弦网格。
    const Int_t theta1ScanN = 500;
    const Int_t T1ScanN = 180;
    const Int_t T2ScanN = 180;

    const Double_t reactionParameters[9] = {
        massA,
        massT,
        massB,
        mass1,
        mass2,
        beamEnergyPerU,
        massNumberA,
        massNumber1,
        massNumber2
    };

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

    TH2D* histogram =
        (TH2D*)histogramDirectory->Get("hTheta1_Theta2");
    if (!histogram) {
        std::cout << "[ERROR] Cannot find hTheta1_Theta2" << std::endl;
        inputFile->Close();
        return;
    }

    histogram->SetTitle("");
    histogram->SetStats(kFALSE);
    histogram->GetXaxis()->SetRangeUser(theta2Min, theta2Max);
    histogram->GetYaxis()->SetRangeUser(theta1Min, theta1Max);
    histogram->GetXaxis()->SetTitle("#theta_{2}^{lab} [deg]");
    histogram->GetYaxis()->SetTitle("#theta_{1}^{lab} [deg]");

    TGraph* boundary = CreateTheta1Theta2BoundaryGraph(
        theta1Min,
        theta1Max,
        theta2Min,
        theta2Max,
        theta1ScanN,
        T1ScanN,
        T2ScanN,
        reactionParameters);

    TMultiGraph* alphaProtonElastic = TwoBodyKinematicsAngleAngle(
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

    TCanvas* canvas = new TCanvas(
        "canvasTheta1Theta2Boundary",
        "Three-body theta1-theta2 boundary",
        900,
        700);

    histogram->Draw("COLZ");
    if (boundary && boundary->GetN() > 2) boundary->Draw("L SAME");
    DrawAngleAngleGraphCollection(alphaProtonElastic);

    TLegend* legend = new TLegend(0.54, 0.77, 0.86, 0.91);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    if (boundary && boundary->GetN() > 2) {
        legend->AddEntry(boundary, "Three-body boundary: F_{12}=0", "l");
    }

    TGraph* elasticLegendGraph =
        GetFirstAngleAngleGraph(alphaProtonElastic);
    if (elasticLegendGraph) {
        legend->AddEntry(
            elasticLegendGraph,
            "^{4}He(p,p)^{4}He, 150 MeV/u",
            "l");
    }

    legend->Draw();
    canvas->SaveAs("output/Theta1Theta2.pdf");

    Int_t boundaryPointCount = 0;
    if (boundary) boundaryPointCount = boundary->GetN();
    std::cout << "Boundary points: " << boundaryPointCount << std::endl;

    inputFile->Close();
}
