// ThreeBodyT1T2Boundary.C
//
// 功能：为三体反应 A + T -> B + 1 + 2 绘制 T1-T2、T1-TB 和 TB-T2
//       三组动能关联边界；每组分别输出纯边界图和蒙特卡洛分布叠加边界图，
//       共生成 output/ 目录下的六个无顶部标题、无统计框 PDF。
//
// 四种候选边界：
//       F_A = p_A - p_1 - p_2 - p_B = 0
//       F_1 = p_1 - p_A - p_2 - p_B = 0
//       F_2 = p_2 - p_A - p_1 - p_B = 0
//       F_B = p_B - p_A - p_1 - p_2 = 0
//
// 方法：
//       不再使用 TF2 + contour。
//       对横坐标逐点扫描，在每一个固定横坐标下求 F(x,y)=0 的所有根。
//       y 方向采用余弦非均匀网格，在物理区域的两个端点附近自动加密，
//       因而可以解析非常靠近 T1=0、T2=0 或 TB=0 的窄边界。
//       找到根以后使用二分法精确求根，最后用 TGraph 连接上下两支边界。
//       含 T1 的关联图统一固定 T1 为横轴。
//
// 单位：
//       输入质量：u (AMU)
//       输入束流动能：MeV/u
//       图中动能：MeV/u
//       程序内部质量：MeV/c^2
//       程序内部动能：总动能 MeV
//
// 注意事项：输入 ROOT 文件的动能坐标为总动能 MeV，本宏按质量数换算为
//           MeV/u；hTB_T1 保留原对象名，但其坐标顺序为 x=T1、y=TB。

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "TAxis.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TMath.h"
#include "TStyle.h"
#include "TSystem.h"


// ============================================================================
// 三体动能-动能边界方程
//
// correlationIndex = 0 : x = T1, y = T2，消去 TB
// correlationIndex = 1 : x = T1, y = TB，消去 T2
// correlationIndex = 2 : x = TB, y = T2，消去 T1
//
// boundaryIndex = 0 : pA = p1 + p2 + pB
// boundaryIndex = 1 : p1 = pA + p2 + pB
// boundaryIndex = 2 : p2 = pA + p1 + pB
// boundaryIndex = 3 : pB = pA + p1 + p2
// ============================================================================

Double_t ThreeBodyBoundaryEquationEnergyEnergy(Double_t x, Double_t y, Int_t boundaryIndex, Int_t correlationIndex, const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;

    const Double_t beamEnergyPerU = par[5];
    const Double_t massNumberA = par[6];
    const Double_t massNumberB = par[7];
    const Double_t massNumber1 = par[8];
    const Double_t massNumber2 = par[9];

    const Double_t TA = massNumberA * beamEnergyPerU;
    const Double_t QValue = mA + mT - mB - m1 - m2;

    Double_t TB = 0.0;
    Double_t T1 = 0.0;
    Double_t T2 = 0.0;

    if (correlationIndex == 0) {
        T1 = massNumber1 * x;
        T2 = massNumber2 * y;
        TB = TA + QValue - T1 - T2;
    } else if (correlationIndex == 1) {
        T1 = massNumber1 * x;
        TB = massNumberB * y;
        T2 = TA + QValue - TB - T1;
    } else if (correlationIndex == 2) {
        TB = massNumberB * x;
        T2 = massNumber2 * y;
        T1 = TA + QValue - TB - T2;
    } else {
        return std::numeric_limits<Double_t>::quiet_NaN();
    }

    if (TB < 0.0 || T1 < 0.0 || T2 < 0.0) return std::numeric_limits<Double_t>::quiet_NaN();

    const Double_t pA = std::sqrt(TA * (TA + 2.0 * mA));
    const Double_t pB = std::sqrt(TB * (TB + 2.0 * mB));
    const Double_t p1 = std::sqrt(T1 * (T1 + 2.0 * m1));
    const Double_t p2 = std::sqrt(T2 * (T2 + 2.0 * m2));

    if (boundaryIndex == 0) return pA - p1 - p2 - pB;
    if (boundaryIndex == 1) return p1 - pA - p2 - pB;
    if (boundaryIndex == 2) return p2 - pA - p1 - pB;
    if (boundaryIndex == 3) return pB - pA - p1 - p2;

    return std::numeric_limits<Double_t>::quiet_NaN();
}


// ============================================================================
// 对给定横坐标 x，计算纵坐标 y 的物理最大值
// ============================================================================

Double_t GetPhysicalYMaximum(Double_t x, Int_t correlationIndex, const Double_t* par)
{
    constexpr Double_t AmuToMeV = 931.49410242;

    const Double_t mA = par[0] * AmuToMeV;
    const Double_t mT = par[1] * AmuToMeV;
    const Double_t mB = par[2] * AmuToMeV;
    const Double_t m1 = par[3] * AmuToMeV;
    const Double_t m2 = par[4] * AmuToMeV;

    const Double_t TA = par[6] * par[5];
    const Double_t QValue = mA + mT - mB - m1 - m2;
    const Double_t availableEnergy = TA + QValue;

    if (correlationIndex == 0) return (availableEnergy - par[8] * x) / par[9]; // T1-T2
    if (correlationIndex == 1) return (availableEnergy - par[8] * x) / par[7]; // T1-TB
    if (correlationIndex == 2) return (availableEnergy - par[7] * x) / par[9]; // TB-T2

    return -1.0;
}


// ============================================================================
// 二分法求一个已经被括住的 F(x,y)=0 根
// ============================================================================

Double_t SolveBoundaryRootBisection(Double_t x, Double_t yLow, Double_t yHigh, Int_t boundaryIndex, Int_t correlationIndex, const Double_t* par)
{
    Double_t fLow = ThreeBodyBoundaryEquationEnergyEnergy(x, yLow, boundaryIndex, correlationIndex, par);
    Double_t fHigh = ThreeBodyBoundaryEquationEnergyEnergy(x, yHigh, boundaryIndex, correlationIndex, par);

    if (!std::isfinite(fLow) || !std::isfinite(fHigh)) return std::numeric_limits<Double_t>::quiet_NaN();
    if (std::abs(fLow) < 1e-10) return yLow;
    if (std::abs(fHigh) < 1e-10) return yHigh;
    if ((fLow > 0.0) == (fHigh > 0.0)) return std::numeric_limits<Double_t>::quiet_NaN();

    for (Int_t iteration = 0; iteration < 100; ++iteration) {
        const Double_t yMiddle = 0.5 * (yLow + yHigh);
        const Double_t fMiddle = ThreeBodyBoundaryEquationEnergyEnergy(x, yMiddle, boundaryIndex, correlationIndex, par);

        if (!std::isfinite(fMiddle)) return std::numeric_limits<Double_t>::quiet_NaN();
        if (std::abs(fMiddle) < 1e-10 || std::abs(yHigh - yLow) < 1e-10) return yMiddle;

        if ((fLow > 0.0) == (fMiddle > 0.0)) {
            yLow = yMiddle;
            fLow = fMiddle;
        } else {
            yHigh = yMiddle;
            fHigh = fMiddle;
        }
    }

    return 0.5 * (yLow + yHigh);
}


// ============================================================================
// 在固定 x 下寻找 F(x,y)=0 的所有根
//
// 使用余弦网格：
// y = ymin + (ymax-ymin)/2 * [1-cos(pi*t)]
//
// 这样会自动在 ymin 和 ymax 附近大幅加密。
// 对 TB-T2 而言，即使根距离 T1=0 边界只有 ~0.01 MeV/u，也能够找到。
// ============================================================================

std::vector<Double_t> FindBoundaryRootsAtX(Double_t x, Int_t boundaryIndex, Int_t correlationIndex, Double_t yMin, Double_t yMax, Int_t scanNY, const Double_t* par)
{
    std::vector<Double_t> roots;

    Double_t physicalYMax = GetPhysicalYMaximum(x, correlationIndex, par);
    if (physicalYMax < yMin) return roots;

    const Double_t upperY = std::min(yMax, physicalYMax);
    if (upperY < yMin) return roots;

    Double_t previousY = yMin;
    Double_t previousF = ThreeBodyBoundaryEquationEnergyEnergy(x, previousY, boundaryIndex, correlationIndex, par);

    if (std::isfinite(previousF) && std::abs(previousF) < 1e-9) roots.push_back(previousY);

    for (Int_t i = 1; i <= scanNY; ++i) {
        const Double_t t = static_cast<Double_t>(i) / scanNY;
        const Double_t currentY = yMin + 0.5 * (upperY - yMin) * (1.0 - std::cos(TMath::Pi() * t));
        const Double_t currentF = ThreeBodyBoundaryEquationEnergyEnergy(x, currentY, boundaryIndex, correlationIndex, par);

        if (std::isfinite(previousF) && std::isfinite(currentF)) {
            if ((previousF > 0.0) != (currentF > 0.0)) {
                const Double_t root = SolveBoundaryRootBisection(x, previousY, currentY, boundaryIndex, correlationIndex, par);
                if (std::isfinite(root) && (roots.empty() || std::abs(root - roots.back()) > 1e-6)) roots.push_back(root);
            } else if (std::abs(currentF) < 1e-9) {
                if (roots.empty() || std::abs(currentY - roots.back()) > 1e-6) roots.push_back(currentY);
            }
        }

        previousY = currentY;
        previousF = currentF;
    }

    std::sort(roots.begin(), roots.end());
    return roots;
}


// ============================================================================
// 构造一条候选边界的 TGraph
//
// 对每个 x 通常有两个根：
//      y_lower(x)
//      y_upper(x)
//
// 先沿 lower 分支从左向右，再沿 upper 分支从右向左，最终形成闭合曲线。
// 如果这个反应不存在该候选边界，返回的 TGraph 中没有点。
// ============================================================================

TGraph* CreateThreeBodyEnergyBoundaryGraph(const char* graphName, Int_t boundaryIndex, Int_t correlationIndex, Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, const Double_t* par, Int_t color, Int_t scanNX, Int_t scanNY)
{
    std::vector<Double_t> xValues;
    std::vector<Double_t> lowerRoots;
    std::vector<Double_t> upperRoots;

    for (Int_t i = 0; i <= scanNX; ++i) {
        const Double_t x = xMin + (xMax - xMin) * i / scanNX;
        const std::vector<Double_t> roots = FindBoundaryRootsAtX(x, boundaryIndex, correlationIndex, yMin, yMax, scanNY, par);

        if (roots.empty()) continue;

        xValues.push_back(x);
        lowerRoots.push_back(roots.front());
        upperRoots.push_back(roots.back());
    }

    TGraph* graph = new TGraph();
    graph->SetName(graphName);
    graph->SetLineColor(color);
    graph->SetLineWidth(3);

    if (xValues.empty()) return graph;

    Bool_t hasTwoBranches = kFALSE;
    for (size_t i = 0; i < xValues.size(); ++i) {
        if (std::abs(upperRoots[i] - lowerRoots[i]) > 1e-6) {
            hasTwoBranches = kTRUE;
            break;
        }
    }

    Int_t pointIndex = 0;

    for (size_t i = 0; i < xValues.size(); ++i) graph->SetPoint(pointIndex++, xValues[i], lowerRoots[i]);

    if (hasTwoBranches) {
        for (Int_t i = static_cast<Int_t>(xValues.size()) - 1; i >= 0; --i) graph->SetPoint(pointIndex++, xValues[i], upperRoots[i]);
        graph->SetPoint(pointIndex++, xValues.front(), lowerRoots.front());
    }

    return graph;
}


// ============================================================================
// 绘制实际存在的候选边界
// ============================================================================

void DrawExistingEnergyBoundaries(TGraph* gA, TGraph* g1, TGraph* g2, TGraph* gB)
{
    if (gA && gA->GetN() > 1) gA->Draw("L SAME");
    if (g1 && g1->GetN() > 1) g1->Draw("L SAME");
    if (g2 && g2->GetN() > 1) g2->Draw("L SAME");
    if (gB && gB->GetN() > 1) gB->Draw("L SAME");
}


// ============================================================================
// 配置 Monte Carlo 动能关联直方图
// ============================================================================

void ConfigureEnergyCorrelationHistogram(TH2D* histogram, Double_t xMassNumber, Double_t yMassNumber, Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, const char* xTitle, const char* yTitle)
{
    histogram->SetTitle("");
    histogram->GetXaxis()->SetLimits(histogram->GetXaxis()->GetXmin() / xMassNumber, histogram->GetXaxis()->GetXmax() / xMassNumber);
    histogram->GetYaxis()->SetLimits(histogram->GetYaxis()->GetXmin() / yMassNumber, histogram->GetYaxis()->GetXmax() / yMassNumber);
    histogram->GetXaxis()->SetRangeUser(xMin, xMax);
    histogram->GetYaxis()->SetRangeUser(yMin, yMax);
    histogram->GetXaxis()->SetTitle(xTitle);
    histogram->GetYaxis()->SetTitle(yTitle);
}


// ============================================================================
// 配置纯边界图坐标轴
// ============================================================================

void ConfigureEnergyBoundaryFrame(TH2D* frame, const char* xTitle, const char* yTitle)
{
    frame->SetTitle("");
    frame->GetXaxis()->SetTitle(xTitle);
    frame->GetYaxis()->SetTitle(yTitle);
    frame->GetXaxis()->CenterTitle();
    frame->GetYaxis()->CenterTitle();
    frame->GetXaxis()->SetTitleSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.05);
    frame->GetXaxis()->SetLabelSize(0.045);
    frame->GetYaxis()->SetLabelSize(0.045);
}


// ============================================================================
// 保存纯边界图和 Monte Carlo + 边界叠加图
// ============================================================================

void SaveEnergyBoundaryFigures(TCanvas* canvas, TH2D* frame, TH2D* histogram, TGraph* gA, TGraph* g1, TGraph* g2, TGraph* gB, const char* boundaryFileName, const char* overlayFileName)
{
    canvas->Clear();
    canvas->SetRightMargin(0.05);
    frame->Draw();
    DrawExistingEnergyBoundaries(gA, g1, g2, gB);
    canvas->Modified();
    canvas->Update();
    canvas->SaveAs(boundaryFileName);

    canvas->Clear();
    canvas->SetRightMargin(0.14);
    histogram->Draw("COLZ");
    DrawExistingEnergyBoundaries(gA, g1, g2, gB);
    canvas->Modified();
    canvas->Update();
    canvas->SaveAs(overlayFileName);
}


// ============================================================================
// 主函数
// ============================================================================

void ThreeBodyBoundaryT1T2()
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
    const Double_t massNumberB = 6.0;
    const Double_t massNumber1 = 1.0;
    const Double_t massNumber2 = 4.0;

    // ------------------------------------------------------------------------
    // 绘图范围，MeV/u
    // ------------------------------------------------------------------------

    const Double_t T1Min = 0.0;
    const Double_t T1Max = 800.0;

    const Double_t T2Min = 0.0;
    const Double_t T2Max = 350.0;

    const Double_t TBMin = 0.0;

    // ------------------------------------------------------------------------
    // 一维求根扫描精度
    //
    // boundaryScanNX：沿横轴扫描多少点
    // boundaryScanNY：每一个横坐标下沿纵轴扫描多少点
    //
    // y 使用余弦非均匀网格，因此 2000 已经能够很好地处理靠近 T=0 的窄分支。
    // ------------------------------------------------------------------------

    const Int_t boundaryScanNX = 3000;
    const Int_t boundaryScanNY = 2000;

    // ------------------------------------------------------------------------
    // Monte Carlo 输入
    // ------------------------------------------------------------------------

    TFile* inputFile = TFile::Open("output/PureThreeBodyKinematics.root", "READ");
    if (!inputFile || inputFile->IsZombie()) return;

    TDirectory* histogramDirectory = (TDirectory*)inputFile->Get("histograms");
    if (!histogramDirectory) return;

    TH2D* hT1T2 = (TH2D*)histogramDirectory->Get("hT1_T2");
    TH2D* hT1TB = (TH2D*)histogramDirectory->Get("hTB_T1");
    TH2D* hTBT2 = (TH2D*)histogramDirectory->Get("hTB_T2");

    if (!hT1T2 || !hT1TB || !hTBT2) return;

    const Double_t TBMax = hT1TB->GetYaxis()->GetXmax() / massNumberB;

    // ------------------------------------------------------------------------
    // 设置 Monte Carlo 直方图
    // ------------------------------------------------------------------------

    ConfigureEnergyCorrelationHistogram(hT1T2, massNumber1, massNumber2, T1Min, T1Max, T2Min, T2Max, "T_{1}^{lab} [MeV/u]", "T_{2}^{lab} [MeV/u]");
    ConfigureEnergyCorrelationHistogram(hT1TB, massNumber1, massNumberB, T1Min, T1Max, TBMin, TBMax, "T_{1}^{lab} [MeV/u]", "T_{B}^{lab} [MeV/u]");
    ConfigureEnergyCorrelationHistogram(hTBT2, massNumberB, massNumber2, TBMin, TBMax, T2Min, T2Max, "T_{B}^{lab} [MeV/u]", "T_{2}^{lab} [MeV/u]");

    // ------------------------------------------------------------------------
    // 反应参数
    //
    // par[0] = massA
    // par[1] = massT
    // par[2] = massB
    // par[3] = mass1
    // par[4] = mass2
    // par[5] = beamEnergyPerU
    // par[6] = massNumberA
    // par[7] = massNumberB
    // par[8] = massNumber1
    // par[9] = massNumber2
    // ------------------------------------------------------------------------

    const Double_t reactionParameters[10] = {
        massA,
        massT,
        massB,
        mass1,
        mass2,
        beamEnergyPerU,
        massNumberA,
        massNumberB,
        massNumber1,
        massNumber2
    };

    // ------------------------------------------------------------------------
    // T1-T2 四种候选边界
    // ------------------------------------------------------------------------

    TGraph* gA_T1T2 = CreateThreeBodyEnergyBoundaryGraph("gA_T1T2", 0, 0, T1Min, T1Max, T2Min, T2Max, reactionParameters, kRed + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g1_T1T2 = CreateThreeBodyEnergyBoundaryGraph("g1_T1T2", 1, 0, T1Min, T1Max, T2Min, T2Max, reactionParameters, kBlue + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g2_T1T2 = CreateThreeBodyEnergyBoundaryGraph("g2_T1T2", 2, 0, T1Min, T1Max, T2Min, T2Max, reactionParameters, kGreen + 2, boundaryScanNX, boundaryScanNY);
    TGraph* gB_T1T2 = CreateThreeBodyEnergyBoundaryGraph("gB_T1T2", 3, 0, T1Min, T1Max, T2Min, T2Max, reactionParameters, kMagenta + 1, boundaryScanNX, boundaryScanNY);

    // ------------------------------------------------------------------------
    // T1-TB 四种候选边界
    // ------------------------------------------------------------------------

    TGraph* gA_T1TB = CreateThreeBodyEnergyBoundaryGraph("gA_T1TB", 0, 1, T1Min, T1Max, TBMin, TBMax, reactionParameters, kRed + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g1_T1TB = CreateThreeBodyEnergyBoundaryGraph("g1_T1TB", 1, 1, T1Min, T1Max, TBMin, TBMax, reactionParameters, kBlue + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g2_T1TB = CreateThreeBodyEnergyBoundaryGraph("g2_T1TB", 2, 1, T1Min, T1Max, TBMin, TBMax, reactionParameters, kGreen + 2, boundaryScanNX, boundaryScanNY);
    TGraph* gB_T1TB = CreateThreeBodyEnergyBoundaryGraph("gB_T1TB", 3, 1, T1Min, T1Max, TBMin, TBMax, reactionParameters, kMagenta + 1, boundaryScanNX, boundaryScanNY);

    // ------------------------------------------------------------------------
    // TB-T2 四种候选边界
    // ------------------------------------------------------------------------

    TGraph* gA_TBT2 = CreateThreeBodyEnergyBoundaryGraph("gA_TBT2", 0, 2, TBMin, TBMax, T2Min, T2Max, reactionParameters, kRed + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g1_TBT2 = CreateThreeBodyEnergyBoundaryGraph("g1_TBT2", 1, 2, TBMin, TBMax, T2Min, T2Max, reactionParameters, kBlue + 1, boundaryScanNX, boundaryScanNY);
    TGraph* g2_TBT2 = CreateThreeBodyEnergyBoundaryGraph("g2_TBT2", 2, 2, TBMin, TBMax, T2Min, T2Max, reactionParameters, kGreen + 2, boundaryScanNX, boundaryScanNY);
    TGraph* gB_TBT2 = CreateThreeBodyEnergyBoundaryGraph("gB_TBT2", 3, 2, TBMin, TBMax, T2Min, T2Max, reactionParameters, kMagenta + 1, boundaryScanNX, boundaryScanNY);

    // ------------------------------------------------------------------------
    // Canvas
    // ------------------------------------------------------------------------

    TCanvas* canvas = new TCanvas("canvasEnergyBoundaries", "Three-body energy-energy boundaries", 900, 800);
    canvas->SetLeftMargin(0.13);
    canvas->SetBottomMargin(0.12);
    canvas->SetTopMargin(0.05);

    // ------------------------------------------------------------------------
    // 空坐标框
    // ------------------------------------------------------------------------

    TH2D* frameT1T2 = new TH2D("frameT1T2Boundaries", "", 100, T1Min, T1Max, 100, T2Min, T2Max);
    TH2D* frameT1TB = new TH2D("frameT1TBBoundaries", "", 100, T1Min, T1Max, 100, TBMin, TBMax);
    TH2D* frameTBT2 = new TH2D("frameT2TBBoundaries", "", 100, TBMin, TBMax, 100, T2Min, T2Max);

    ConfigureEnergyBoundaryFrame(frameT1T2, "T_{1}^{lab} [MeV/u]", "T_{2}^{lab} [MeV/u]");
    ConfigureEnergyBoundaryFrame(frameT1TB, "T_{1}^{lab} [MeV/u]", "T_{B}^{lab} [MeV/u]");
    ConfigureEnergyBoundaryFrame(frameTBT2, "T_{B}^{lab} [MeV/u]", "T_{2}^{lab} [MeV/u]");

    // ------------------------------------------------------------------------
    // 输出六个 PDF
    // ------------------------------------------------------------------------

    SaveEnergyBoundaryFigures(canvas, frameT1T2, hT1T2, gA_T1T2, g1_T1T2, g2_T1T2, gB_T1T2, "output/T1T2Boundaries1.pdf", "output/T1T2Boundaries2.pdf");

    SaveEnergyBoundaryFigures(canvas, frameT1TB, hT1TB, gA_T1TB, g1_T1TB, g2_T1TB, gB_T1TB, "output/T1TBBoundaries1.pdf", "output/T1TBBoundaries2.pdf");

    SaveEnergyBoundaryFigures(canvas, frameTBT2, hTBT2, gA_TBT2, g1_TBT2, g2_TBT2, gB_TBT2, "output/T2TBBoundaries1.pdf", "output/T2TBBoundaries2.pdf");

    inputFile->Close();
}
