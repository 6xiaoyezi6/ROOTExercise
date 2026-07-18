#include <algorithm>
#include <cmath>
#include <iostream>

#include "TCanvas.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TStyle.h"
#include "TTree.h"

// 自动设置一维直方图坐标轴标题，并在纵轴中显示 bin 宽度。
void SetHistAxisTitle(TH1* hist, const char* xTitle, const char* xUnit)
{
    double binWidth = hist->GetXaxis()->GetBinWidth(1);

    if (xUnit[0] == '\0') {
        hist->GetXaxis()->SetTitle(xTitle);
        hist->GetYaxis()->SetTitle(Form("Counts / %.3g", binWidth));
    } else {
        hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
        hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
    }
}

// 调整两个叠加直方图的完整统计框，避免互相遮挡。
void MoveStatsBoxes(TH1* leastHist, TH1* kalmanHist)
{
    TPaveStats* leastStats =
        (TPaveStats*)leastHist->GetListOfFunctions()->FindObject("stats");
    TPaveStats* kalmanStats =
        (TPaveStats*)kalmanHist->GetListOfFunctions()->FindObject("stats");

    if (leastStats) {
        leastStats->SetX1NDC(0.64);
        leastStats->SetX2NDC(0.89);
        leastStats->SetY1NDC(0.69);
        leastStats->SetY2NDC(0.89);
        leastStats->SetTextColor(kRed + 1);
    }

    if (kalmanStats) {
        kalmanStats->SetX1NDC(0.64);
        kalmanStats->SetX2NDC(0.89);
        kalmanStats->SetY1NDC(0.47);
        kalmanStats->SetY2NDC(0.67);
        kalmanStats->SetTextColor(kBlue + 1);
    }
}

// 比较最小二乘和 Kalman 拟合参数相对于真实参数的偏差。
void CompareDriftChamberFits()
{
    gStyle->SetOptStat(111111111);
    gStyle->SetPaperSize(29.7, 21.0);
    TH1::StatOverflows(kTRUE);

    TFile* inputFile = TFile::Open("DriftChamberData.root", "READ");
    TFile* leastFile = TFile::Open("FitDriftChamberData.root", "READ");
    TFile* kalmanFile = TFile::Open("KalmanitDriftChamberData.root", "READ");

    TTree* inputTree = (TTree*)inputFile->Get("tree");
    TTree* leastTree = (TTree*)leastFile->Get("fitTree");
    TTree* kalmanTree = (TTree*)kalmanFile->Get("kalmanTree");

    int eventID = 0;
    int leastEventID = 0;
    int kalmanEventID = 0;
    double trueSlope = 0.0;
    double trueIntercept = 0.0;
    double leastA = -999.0;
    double leastB = -999.0;
    double kalmanA = -999.0;
    double kalmanB = -999.0;

    inputTree->SetBranchAddress("eventID", &eventID);
    inputTree->SetBranchAddress("trueSlope", &trueSlope);
    inputTree->SetBranchAddress("trueIntercept", &trueIntercept);
    leastTree->SetBranchAddress("eventID", &leastEventID);
    leastTree->SetBranchAddress("a", &leastA);
    leastTree->SetBranchAddress("b", &leastB);
    kalmanTree->SetBranchAddress("eventID", &kalmanEventID);
    kalmanTree->SetBranchAddress("a", &kalmanA);
    kalmanTree->SetBranchAddress("b", &kalmanB);

    TFile* outputFile = new TFile("CompareDriftChamberFits.root", "RECREATE");
    TDirectoryFile* histDir =
        (TDirectoryFile*)outputFile->mkdir("histograms");
    histDir->cd();

    TH1D* hLeastA = new TH1D(
        "hLeastA", "Slope deviation", 150, -1.5, 1.5);
    TH1D* hKalmanA = new TH1D(
        "hKalmanA", "Slope deviation", 150, -1.5, 1.5);
    TH1D* hLeastB = new TH1D(
        "hLeastB", "Intercept deviation", 150, -30.0, 30.0);
    TH1D* hKalmanB = new TH1D(
        "hKalmanB", "Intercept deviation", 150, -30.0, 30.0);

    SetHistAxisTitle(hLeastA, "a - trueSlope", "");
    SetHistAxisTitle(hKalmanA, "a - trueSlope", "");
    SetHistAxisTitle(hLeastB, "b - trueIntercept", "mm");
    SetHistAxisTitle(hKalmanB, "b - trueIntercept", "mm");

    hLeastA->SetLineColor(kRed + 1);
    hLeastA->SetLineWidth(3);
    hKalmanA->SetLineColor(kBlue + 1);
    hKalmanA->SetLineStyle(2);
    hKalmanA->SetLineWidth(3);

    hLeastB->SetLineColor(kRed + 1);
    hLeastB->SetLineWidth(3);
    hKalmanB->SetLineColor(kBlue + 1);
    hKalmanB->SetLineStyle(2);
    hKalmanB->SetLineWidth(3);

    Long64_t nEntries = inputTree->GetEntries();
    if (leastTree->GetEntries() < nEntries) {
        nEntries = leastTree->GetEntries();
    }
    if (kalmanTree->GetEntries() < nEntries) {
        nEntries = kalmanTree->GetEntries();
    }

    Long64_t comparedEvents = 0;
    Long64_t mismatchedEvents = 0;
    double maximumADifference = 0.0;
    double maximumBDifference = 0.0;

    for (Long64_t entry = 0; entry < nEntries; ++entry) {
        inputTree->GetEntry(entry);
        leastTree->GetEntry(entry);
        kalmanTree->GetEntry(entry);

        if (eventID != leastEventID || eventID != kalmanEventID) {
            ++mismatchedEvents;
            continue;
        }

        if (leastA == -999.0 || leastB == -999.0 ||
            kalmanA == -999.0 || kalmanB == -999.0) {
            continue;
        }

        hLeastA->Fill(leastA - trueSlope);
        hKalmanA->Fill(kalmanA - trueSlope);
        hLeastB->Fill(leastB - trueIntercept);
        hKalmanB->Fill(kalmanB - trueIntercept);

        double aDifference = std::fabs(leastA - kalmanA);
        double bDifference = std::fabs(leastB - kalmanB);
        if (aDifference > maximumADifference) {
            maximumADifference = aDifference;
        }
        if (bDifference > maximumBDifference) {
            maximumBDifference = bDifference;
        }

        ++comparedEvents;
    }

    double slopeMaximum = std::max(hLeastA->GetMaximum(), hKalmanA->GetMaximum());
    double interceptMaximum = std::max(hLeastB->GetMaximum(), hKalmanB->GetMaximum());
    hLeastA->SetMaximum(1.18 * slopeMaximum);
    hLeastB->SetMaximum(1.18 * interceptMaximum);

    TCanvas* canvas = new TCanvas(
        "compareCanvas", "Drift chamber fitting comparison", 1500, 700);
    canvas->Divide(2, 1);

    canvas->cd(1);
    gPad->SetGrid();
    hLeastA->Draw("HIST");
    hKalmanA->Draw("HIST SAMES");
    TLegend* slopeLegend = new TLegend(0.12, 0.76, 0.47, 0.89);
    slopeLegend->SetBorderSize(0);
    slopeLegend->AddEntry(hLeastA, "Least squares", "l");
    slopeLegend->AddEntry(hKalmanA, "Kalman filter", "l");
    slopeLegend->Draw();
    gPad->Update();
    MoveStatsBoxes(hLeastA, hKalmanA);
    gPad->Modified();

    canvas->cd(2);
    gPad->SetGrid();
    hLeastB->Draw("HIST");
    hKalmanB->Draw("HIST SAMES");
    TLegend* interceptLegend = new TLegend(0.12, 0.76, 0.47, 0.89);
    interceptLegend->SetBorderSize(0);
    interceptLegend->AddEntry(hLeastB, "Least squares", "l");
    interceptLegend->AddEntry(hKalmanB, "Kalman filter", "l");
    interceptLegend->Draw();
    gPad->Update();
    MoveStatsBoxes(hLeastB, hKalmanB);
    gPad->Modified();

    canvas->Update();
    canvas->SaveAs("CompareDriftChamberFits.pdf");

    histDir->cd();
    hLeastA->Write();
    hKalmanA->Write();
    hLeastB->Write();
    hKalmanB->Write();

    std::cout << "Compared events = " << comparedEvents << std::endl;
    std::cout << "Mismatched event IDs = " << mismatchedEvents << std::endl;
    std::cout << "Least squares: mean(delta a) = " << hLeastA->GetMean()
              << ", RMS(delta a) = " << hLeastA->GetRMS() << std::endl;
    std::cout << "Kalman filter: mean(delta a) = " << hKalmanA->GetMean()
              << ", RMS(delta a) = " << hKalmanA->GetRMS() << std::endl;
    std::cout << "Least squares: mean(delta b) = " << hLeastB->GetMean()
              << ", RMS(delta b) = " << hLeastB->GetRMS() << " mm" << std::endl;
    std::cout << "Kalman filter: mean(delta b) = " << hKalmanB->GetMean()
              << ", RMS(delta b) = " << hKalmanB->GetRMS() << " mm" << std::endl;
    std::cout << "Maximum |aLeast - aKalman| = "
              << maximumADifference << std::endl;
    std::cout << "Maximum |bLeast - bKalman| = "
              << maximumBDifference << " mm" << std::endl;

    outputFile->Close();
    inputFile->Close();
    leastFile->Close();
    kalmanFile->Close();
}
