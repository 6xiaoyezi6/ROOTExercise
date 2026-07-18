#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <vector>

#include "TCanvas.h"
#include "TColor.h"
#include "TFile.h"
#include "TGraph.h"
#include "TInterpreter.h"
#include "TMultiGraph.h"
#include "TStyle.h"
#include "TTree.h"

// 按照 x 坐标从小到大排列击中点。
bool CompareHitsByX(const std::tuple<double, double>& left,
                    const std::tuple<double, double>& right)
{
    return std::get<0>(left) < std::get<0>(right);
}

// 使用二维状态 [y, a] 对一个事件执行线性 Kalman 滤波。
bool KalmanFitOneEvent(const std::vector<std::tuple<double, double>>& hits,
                       double measurementVariance,
                       double& a,
                       double& b,
                       double& chi2,
                       std::vector<std::tuple<double, double>>& sortedHits)
{
    sortedHits = hits;
    std::sort(sortedHits.begin(), sortedHits.end(), CompareHitsByX);

    if (sortedHits.size() < 2 || measurementVariance <= 0.0) {
        return false;
    }

    double firstX = std::get<0>(sortedHits.front());
    double lastX = std::get<0>(sortedHits.back());
    if (std::fabs(lastX - firstX) < 1.0e-12) {
        return false;
    }

    double stateY = std::get<1>(sortedHits.front());
    double stateSlope = 0.0;

    double pYY = measurementVariance;
    double pYSlope = 0.0;
    double pSlopeSlope = 1.0e6 * measurementVariance;

    chi2 = 0.0;
    double previousX = firstX;

    for (std::size_t i = 1; i < sortedHits.size(); ++i) {
        double x = std::get<0>(sortedHits[i]);
        double measuredY = std::get<1>(sortedHits[i]);
        double deltaX = x - previousX;

        // 预测：X(k|k-1) = F X(k-1|k-1)。
        double predictedY = stateY + deltaX * stateSlope;
        double predictedSlope = stateSlope;

        double predictedPYY = pYY + 2.0 * deltaX * pYSlope
                            + deltaX * deltaX * pSlopeSlope;
        double predictedPYSlope = pYSlope + deltaX * pSlopeSlope;
        double predictedPSlopeSlope = pSlopeSlope;

        // 更新：利用当前位置测得的 y 修正预测状态。
        double residual = measuredY - predictedY;
        double residualVariance = predictedPYY + measurementVariance;
        if (residualVariance <= 0.0 || !std::isfinite(residualVariance)) {
            return false;
        }

        double gainY = predictedPYY / residualVariance;
        double gainSlope = predictedPYSlope / residualVariance;

        stateY = predictedY + gainY * residual;
        stateSlope = predictedSlope + gainSlope * residual;

        pYY = (1.0 - gainY) * predictedPYY;
        pYSlope = (1.0 - gainY) * predictedPYSlope;
        pSlopeSlope = predictedPSlopeSlope
                    - gainSlope * predictedPYSlope;

        chi2 += residual * residual / residualVariance;
        previousX = x;
    }

    a = stateSlope;
    b = stateY - a * previousX;

    return std::isfinite(a) && std::isfinite(b) && std::isfinite(chi2);
}

// 拟合全部事件并保存结果，同时绘制前三个成功拟合的事件。
void KalmanitDriftChamberData()
{
    gStyle->SetOptStat(111111111);
    gInterpreter->GenerateDictionary("vector<tuple<double,double>>", "vector;tuple");

    TFile* inputFile = TFile::Open("DriftChamberData.root", "READ");
    TTree* tree = (TTree*)inputFile->Get("tree");

    int eventID = 0;
    int nHits = 0;
    std::vector<std::tuple<double, double>>* hits = 0;
    tree->SetBranchAddress("eventID", &eventID);
    tree->SetBranchAddress("nHits", &nHits);
    tree->SetBranchAddress("x1", &hits);

    double a = -999.0;
    double b = -999.0;
    double chi2 = -999.0;

    TFile* outputFile = new TFile("KalmanitDriftChamberData.root", "RECREATE");
    TTree* kalmanTree = new TTree("kalmanTree", "Kalman fitting results");
    kalmanTree->Branch("eventID", &eventID);
    kalmanTree->Branch("nHits", &nHits);
    kalmanTree->Branch("a", &a);
    kalmanTree->Branch("b", &b);
    kalmanTree->Branch("chi2", &chi2);

    const double measurementVariance = 7.0;
    const int nPlots = 3;
    const int colors[nPlots] = {kRed + 1, kBlue + 1, kGreen + 2};
    TGraph* pointGraphs[nPlots] = {0};
    TGraph* lineGraphs[nPlots] = {0};

    int successfulFits = 0;
    int plotCount = 0;
    Long64_t nEntries = tree->GetEntries();

    for (Long64_t entry = 0; entry < nEntries; ++entry) {
        tree->GetEntry(entry);

        a = -999.0;
        b = -999.0;
        chi2 = -999.0;
        std::vector<std::tuple<double, double>> sortedHits;

        bool fitted = KalmanFitOneEvent(*hits, measurementVariance,
                                        a, b, chi2, sortedHits);

        if (!fitted) {
            a = -999.0;
            b = -999.0;
            chi2 = -999.0;
        }

        if (fitted) {
            ++successfulFits;

            if (plotCount < nPlots) {
                pointGraphs[plotCount] = new TGraph(sortedHits.size());
                for (std::size_t i = 0; i < sortedHits.size(); ++i) {
                    pointGraphs[plotCount]->SetPoint(
                        i,
                        std::get<0>(sortedHits[i]),
                        std::get<1>(sortedHits[i]));
                }
                pointGraphs[plotCount]->SetMarkerColor(colors[plotCount]);
                pointGraphs[plotCount]->SetMarkerStyle(20 + plotCount);
                pointGraphs[plotCount]->SetMarkerSize(1.2);

                lineGraphs[plotCount] = new TGraph(2);
                lineGraphs[plotCount]->SetPoint(0, -50.0, a * -50.0 + b);
                lineGraphs[plotCount]->SetPoint(1, 50.0, a * 50.0 + b);
                lineGraphs[plotCount]->SetLineColor(colors[plotCount]);
                lineGraphs[plotCount]->SetLineWidth(2);

                ++plotCount;
            }
        }

        kalmanTree->Fill();
    }

    outputFile->cd();
    kalmanTree->Write();
    outputFile->Close();

    if (plotCount == 0) {
        std::cerr << "No event was fitted successfully" << std::endl;
        inputFile->Close();
        return;
    }

    TMultiGraph* graph = new TMultiGraph(
        "kalmanGraph",
        "First three Kalman-fitted drift-chamber events;x (mm);y (mm)");

    for (int i = 0; i < plotCount; ++i) {
        graph->Add(lineGraphs[i], "L");
    }
    for (int i = 0; i < plotCount; ++i) {
        graph->Add(pointGraphs[i], "P");
    }

    TCanvas* canvas = new TCanvas(
        "kalmanCanvas", "Kalman drift chamber fits", 1100, 800);
    canvas->SetGrid();
    graph->Draw("A");
    canvas->Update();
    canvas->SaveAs("KalmanitDriftChamberData.pdf");

    std::cout << "Total events = " << nEntries << std::endl;
    std::cout << "Successfully fitted events = " << successfulFits << std::endl;
    inputFile->Close();
}
