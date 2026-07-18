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

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.cc"

// 递归读取当前事件的全部坐标对，并在读取完成后进行直线拟合。
bool FitOneEventRecursive(const std::vector<std::tuple<double, double>>& hits,
                          std::size_t index,//index 控制递归进行到哪一步，表示当前正在处理 hits 中的第几个坐标点
                          std::vector<double>& x,
                          std::vector<double>& y,
                          double& a,
                          double& b,
                          double& chi2)
{
    ////递归终止条件
    if (index == hits.size()) 
    {
        return LeastSquares(x, y, a, b, chi2);
    }
    
    x.emplace_back(std::get<0>(hits[index])); //读取元组tuple的第一个元素
    y.emplace_back(std::get<1>(hits[index]));

    return FitOneEventRecursive(hits, index + 1, x, y,a, b, chi2);//每递归调用一次，使用index+1 处理下一个点对
}

// 拟合全部事件，将结果保存到 ROOT 文件，并绘制前三个成功拟合的事件。
void LeastSquaresFitDriftChamberData()
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

    TFile* outputFile = new TFile("FitDriftChamberData.root", "RECREATE");
    TTree* fitTree = new TTree("fitTree", "Drift chamber fitting results");
    fitTree->Branch("eventID", &eventID);
    fitTree->Branch("nHits", &nHits);
    fitTree->Branch("a", &a);
    fitTree->Branch("b", &b);
    fitTree->Branch("chi2", &chi2);

    const int nPlots = 3;
    const int colors[nPlots] = {kRed + 1, kBlue + 1, kGreen + 2};
    TGraph* pointGraphs[nPlots] = {0};
    TGraph* lineGraphs[nPlots] = {0};

    int successfulFits = 0;
    int plotCount = 0;
    Long64_t nEntries = tree->GetEntries();

    for (Long64_t entry = 0; entry < nEntries; ++entry) {
        tree->GetEntry(entry);

        std::vector<double> x;
        std::vector<double> y;
        a = -999.0;
        b = -999.0;
        chi2 = -999.0;

        bool fitted = false;
        // 有两对坐标点(x,y)才进行拟合，两层漂移室至少
        if (hits->size() >= 2) 
        {
            fitted = FitOneEventRecursive(*hits, 0, x, y, a, b, chi2);
        }
        //拟合失败填-999
        if (!fitted) {
            a = -999.0;
            b = -999.0;
            chi2 = -999.0;
        }

        if (fitted) {
            ++successfulFits;

            if (plotCount < nPlots) {
                pointGraphs[plotCount] = new TGraph(x.size());
                for (std::size_t i = 0; i < x.size(); ++i) {
                    pointGraphs[plotCount]->SetPoint(i, x[i], y[i]);
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

        fitTree->Fill();
    }

    outputFile->cd();
    fitTree->Write();
    outputFile->Close();

    if (plotCount == 0) {
        std::cerr << "No event was fitted successfully" << std::endl;
        inputFile->Close();
        return;
    }

    TMultiGraph* graph = new TMultiGraph(
        "graph", "First three fitted drift-chamber events;x (mm);y (mm)");

    for (int i = 0; i < plotCount; ++i) {
        graph->Add(lineGraphs[i], "L");
    }
    for (int i = 0; i < plotCount; ++i) {
        graph->Add(pointGraphs[i], "P");
    }

    TCanvas* canvas = new TCanvas("canvas", "Drift chamber fits", 1100, 800);
    canvas->SetGrid();
    graph->Draw("A");
    canvas->Update();
    canvas->SaveAs("FitDriftChamberData.pdf");

    std::cout << "Total events = " << nEntries << std::endl;
    std::cout << "Successfully fitted events = " << successfulFits << std::endl;
    inputFile->Close();
}
