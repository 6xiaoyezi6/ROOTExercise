#include <cmath>
#include <iostream>
#include <vector>

#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TH2D.h"
#include "TInterpreter.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.cc"

//这里没有使用组合数限制
namespace TestMyLSInternal {

void SetDifferenceHistAxisTitle(TH1 *histogram,
                                const char *xTitle,
                                const char *xUnit)
{
    const Double_t binWidth = histogram->GetXaxis()->GetBinWidth(1);
    histogram->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
    histogram->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void SetComparisonHistAxisTitle(TH2 *histogram,
                                const char *xTitle,
                                const char *yTitle,
                                const char *unit)
{
    histogram->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, unit));
    histogram->GetYaxis()->SetTitle(Form("%s (%s)", yTitle, unit));
}

Bool_t IsXLayer(Int_t layer)
{
    return layer == 0 || layer == 1 || layer == 4 || layer == 5;
}

Bool_t IsYLayer(Int_t layer)
{
    return layer == 2 || layer == 3 || layer == 6 || layer == 7;
}

//把一个 BDC 事件的原始 hit 按层方向拆分为X和Y方向
//构造vector<XLayerHits> &xHits和vector<YLayerHits> &yHits
void BuildBDCHits(Long64_t eventNumber,
                  const std::vector<Int_t> *tdc,
                  const std::vector<Int_t> *layer,
                  const std::vector<Int_t> *wire,
                  const std::vector<Double_t> *drift,
                  const std::vector<Double_t> *coordinate,
                  const std::vector<Double_t> *zcoordinate,
                  std::vector<XLayerHits> &xHits,//这两个参数是引用参数，但是相当于输出参数
                  std::vector<YLayerHits> &yHits)
{
    xHits.clear();
    yHits.clear();

    const Int_t numberOfHits = static_cast<Int_t>(tdc->size());
    for (Int_t i = 0; i < numberOfHits; i++) {
        if (!TMath::Finite((*drift)[i]) || (*drift)[i] < 0.0) {
            continue;
        }

        if (IsXLayer((*layer)[i])) {
            XLayerHits hit;
            hit.EventID = static_cast<Int_t>(eventNumber);
            hit.XLayerID = (*layer)[i];
            hit.HitIndex = i;
            hit.WireID = (*wire)[i];
            hit.TDC = (*tdc)[i];
            hit.XCoordinate = (*coordinate)[i];
            hit.ZCoordinate = (*zcoordinate)[i];
            hit.DriftDistance = (*drift)[i];
            xHits.push_back(hit);
        }
        else if (IsYLayer((*layer)[i])) {
            YLayerHits hit;
            hit.EventID = static_cast<Int_t>(eventNumber);
            hit.YLayerID = (*layer)[i];
            hit.HitIndex = i;
            hit.WireID = (*wire)[i];
            hit.TDC = (*tdc)[i];
            hit.YCoordinate = (*coordinate)[i];
            hit.ZCoordinate = (*zcoordinate)[i];
            hit.DriftDistance = (*drift)[i];
            yHits.push_back(hit);
        }
    }
}

} // namespace TestMyLSInternal

void TestMyLS()
{
    using namespace TestMyLSInternal;

    gInterpreter->GenerateDictionary(
        "EventFitResults",
        "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.h;vector;TVector3.h");

    const TString analysisDirectory ="/Users/yemingxin/ROOT_Exercise/write_from_memory/LeastSquare";
    const TString inputFilename = analysisDirectory + "/run0291_BDC.root";
    const TString outputFilename = analysisDirectory + "/TestMyLS.root";

    TFile *inputFile = TFile::Open(inputFilename.Data(), "READ");
    TTree *rawtree = (TTree *)inputFile->Get("rawtree");
    TTree *calTree = (TTree *)inputFile->Get("CalTreeBDC");

    std::vector<Int_t> *bdc1_tdc = 0;
    std::vector<Int_t> *bdc1_layer = 0;
    std::vector<Int_t> *bdc1_wire = 0;
    std::vector<Double_t> *bdc1_drift = 0;
    std::vector<Double_t> *bdc1_xcoordinate = 0;
    std::vector<Double_t> *bdc1_zcoordinate = 0;
    std::vector<Int_t> *bdc2_tdc = 0;
    std::vector<Int_t> *bdc2_layer = 0;
    std::vector<Int_t> *bdc2_wire = 0;
    std::vector<Double_t> *bdc2_drift = 0;
    std::vector<Double_t> *bdc2_xcoordinate = 0;
    std::vector<Double_t> *bdc2_zcoordinate = 0;

    rawtree->SetBranchAddress("bdc1_tdc", &bdc1_tdc);
    rawtree->SetBranchAddress("bdc1_layer", &bdc1_layer);
    rawtree->SetBranchAddress("bdc1_wire", &bdc1_wire);
    rawtree->SetBranchAddress("bdc1_drift", &bdc1_drift);
    rawtree->SetBranchAddress("bdc1_xcoordinate", &bdc1_xcoordinate);
    rawtree->SetBranchAddress("bdc1_zcoordinate", &bdc1_zcoordinate);
    rawtree->SetBranchAddress("bdc2_tdc", &bdc2_tdc);
    rawtree->SetBranchAddress("bdc2_layer", &bdc2_layer);
    rawtree->SetBranchAddress("bdc2_wire", &bdc2_wire);
    rawtree->SetBranchAddress("bdc2_drift", &bdc2_drift);
    rawtree->SetBranchAddress("bdc2_xcoordinate", &bdc2_xcoordinate);
    rawtree->SetBranchAddress("bdc2_zcoordinate", &bdc2_zcoordinate);

    Long64_t EventNumber = -1;
    Double_t BDC1_Anaroot_X = -999.0;
    Double_t BDC1_Anaroot_Y = -999.0;
    Double_t BDC1_Anaroot_ThetaX = -999.0;
    Double_t BDC1_Anaroot_ThetaY = -999.0;
    Double_t BDC2_Anaroot_X = -999.0;
    Double_t BDC2_Anaroot_Y = -999.0;
    Double_t BDC2_Anaroot_ThetaX = -999.0;
    Double_t BDC2_Anaroot_ThetaY = -999.0;

    calTree->SetBranchAddress("EventNumber", &EventNumber);
    calTree->SetBranchAddress("BDC1_X", &BDC1_Anaroot_X);
    calTree->SetBranchAddress("BDC1_Y", &BDC1_Anaroot_Y);
    calTree->SetBranchAddress("BDC1_ThetaX", &BDC1_Anaroot_ThetaX);
    calTree->SetBranchAddress("BDC1_ThetaY", &BDC1_Anaroot_ThetaY);
    calTree->SetBranchAddress("BDC2_X", &BDC2_Anaroot_X);
    calTree->SetBranchAddress("BDC2_Y", &BDC2_Anaroot_Y);
    calTree->SetBranchAddress("BDC2_ThetaX", &BDC2_Anaroot_ThetaX);
    calTree->SetBranchAddress("BDC2_ThetaY", &BDC2_Anaroot_ThetaY);

    TFile *outputFile = new TFile(outputFilename.Data(), "RECREATE");
    TTree *outputTree = new TTree("tree", "MyLS and anaroot BDC track comparison");

    EventFitResults BDC1FitResult;
    EventFitResults BDC2FitResult;
    outputTree->Branch("BDC1FitResult", &BDC1FitResult);
    outputTree->Branch("BDC2FitResult", &BDC2FitResult);

    TH2D *hBDC1XComparison = new TH2D("hBDC1XComparison", "BDC1 X: MyLS vs anaroot", 400, -60.0, 60.0, 400, -60.0, 60.0);
    TH2D *hBDC1YComparison = new TH2D("hBDC1YComparison", "BDC1 Y: MyLS vs anaroot", 400, -60.0, 60.0, 400, -60.0, 60.0);
    TH2D *hBDC1ThetaXComparison = new TH2D("hBDC1ThetaXComparison", "BDC1 ThetaX: MyLS vs anaroot", 400, -0.2, 0.2, 400, -0.2, 0.2);
    TH2D *hBDC1ThetaYComparison = new TH2D("hBDC1ThetaYComparison", "BDC1 ThetaY: MyLS vs anaroot", 400, -0.2, 0.2, 400, -0.2, 0.2);
    TH2D *hBDC2XComparison = new TH2D("hBDC2XComparison", "BDC2 X: MyLS vs anaroot", 400, -60.0, 60.0, 400, -60.0, 60.0);
    TH2D *hBDC2YComparison = new TH2D("hBDC2YComparison", "BDC2 Y: MyLS vs anaroot", 400, -60.0, 60.0, 400, -60.0, 60.0);
    TH2D *hBDC2ThetaXComparison = new TH2D("hBDC2ThetaXComparison", "BDC2 ThetaX: MyLS vs anaroot", 400, -0.2, 0.2, 400, -0.2, 0.2);
    TH2D *hBDC2ThetaYComparison = new TH2D("hBDC2ThetaYComparison", "BDC2 ThetaY: MyLS vs anaroot", 400, -0.2, 0.2, 400, -0.2, 0.2);

    SetComparisonHistAxisTitle(hBDC1XComparison, "anaroot BDC1 X", "MyLS BDC1 X", "mm");
    SetComparisonHistAxisTitle(hBDC1YComparison, "anaroot BDC1 Y", "MyLS BDC1 Y", "mm");
    SetComparisonHistAxisTitle(hBDC1ThetaXComparison, "anaroot BDC1 ThetaX", "MyLS BDC1 ThetaX", "rad");
    SetComparisonHistAxisTitle(hBDC1ThetaYComparison, "anaroot BDC1 ThetaY", "MyLS BDC1 ThetaY", "rad");
    SetComparisonHistAxisTitle(hBDC2XComparison, "anaroot BDC2 X", "MyLS BDC2 X", "mm");
    SetComparisonHistAxisTitle(hBDC2YComparison, "anaroot BDC2 Y", "MyLS BDC2 Y", "mm");
    SetComparisonHistAxisTitle(hBDC2ThetaXComparison, "anaroot BDC2 ThetaX", "MyLS BDC2 ThetaX", "rad");
    SetComparisonHistAxisTitle(hBDC2ThetaYComparison, "anaroot BDC2 ThetaY", "MyLS BDC2 ThetaY", "rad");

    TH1D *hBDC1DeltaX = new TH1D("hBDC1DeltaX", "MyLS BDC1 X minus anaroot BDC1 X", 400, -10.0, 10.0);
    TH1D *hBDC1DeltaY = new TH1D("hBDC1DeltaY", "MyLS BDC1 Y minus anaroot BDC1 Y", 400, -10.0, 10.0);
    TH1D *hBDC1DeltaThetaX = new TH1D("hBDC1DeltaThetaX", "MyLS BDC1 ThetaX minus anaroot BDC1 ThetaX", 400, -0.05, 0.05);
    TH1D *hBDC1DeltaThetaY = new TH1D("hBDC1DeltaThetaY", "MyLS BDC1 ThetaY minus anaroot BDC1 ThetaY", 400, -0.05, 0.05);
    TH1D *hBDC2DeltaX = new TH1D("hBDC2DeltaX", "MyLS BDC2 X minus anaroot BDC2 X", 400, -10.0, 10.0);
    TH1D *hBDC2DeltaY = new TH1D("hBDC2DeltaY", "MyLS BDC2 Y minus anaroot BDC2 Y", 400, -10.0, 10.0);
    TH1D *hBDC2DeltaThetaX = new TH1D("hBDC2DeltaThetaX", "MyLS BDC2 ThetaX minus anaroot BDC2 ThetaX", 400, -0.05, 0.05);
    TH1D *hBDC2DeltaThetaY = new TH1D("hBDC2DeltaThetaY", "MyLS BDC2 ThetaY minus anaroot BDC2 ThetaY", 400, -0.05, 0.05);

    SetDifferenceHistAxisTitle(hBDC1DeltaX, "MyLS BDC1 X - anaroot BDC1 X", "mm");
    SetDifferenceHistAxisTitle(hBDC1DeltaY, "MyLS BDC1 Y - anaroot BDC1 Y", "mm");
    SetDifferenceHistAxisTitle(hBDC1DeltaThetaX, "MyLS BDC1 ThetaX - anaroot BDC1 ThetaX", "rad");
    SetDifferenceHistAxisTitle(hBDC1DeltaThetaY, "MyLS BDC1 ThetaY - anaroot BDC1 ThetaY", "rad");
    SetDifferenceHistAxisTitle(hBDC2DeltaX, "MyLS BDC2 X - anaroot BDC2 X", "mm");
    SetDifferenceHistAxisTitle(hBDC2DeltaY, "MyLS BDC2 Y - anaroot BDC2 Y", "mm");
    SetDifferenceHistAxisTitle(hBDC2DeltaThetaX, "MyLS BDC2 ThetaX - anaroot BDC2 ThetaX", "rad");
    SetDifferenceHistAxisTitle(hBDC2DeltaThetaY, "MyLS BDC2 ThetaY - anaroot BDC2 ThetaY", "rad");

    std::vector<XLayerHits> bdc1XHits;
    std::vector<YLayerHits> bdc1YHits;
    std::vector<XLayerHits> bdc2XHits;
    std::vector<YLayerHits> bdc2YHits;

    const Int_t minimumXLayers = 3;
    const Int_t minimumYLayers = 3;

    Long64_t numberOfEntries = rawtree->GetEntries();
    if (calTree->GetEntries() < numberOfEntries) {
        numberOfEntries = calTree->GetEntries();
    }

    for (Long64_t entry = 0; entry < numberOfEntries; entry++) {
        rawtree->GetEntry(entry);
        calTree->GetEntry(entry);

        BuildBDCHits(EventNumber, bdc1_tdc, bdc1_layer, bdc1_wire,
                     bdc1_drift, bdc1_xcoordinate, bdc1_zcoordinate,
                     bdc1XHits, bdc1YHits);
        BuildBDCHits(EventNumber, bdc2_tdc, bdc2_layer, bdc2_wire,
                     bdc2_drift, bdc2_xcoordinate, bdc2_zcoordinate,
                     bdc2XHits, bdc2YHits);

        const Bool_t bdc1FitSuccess = LeastSquaresWithLeftRightResolveAndMultiHit(
            bdc1XHits, bdc1YHits, BDC1FitResult,
            minimumXLayers, minimumYLayers);
        const Bool_t bdc2FitSuccess = LeastSquaresWithLeftRightResolveAndMultiHit(
            bdc2XHits, bdc2YHits, BDC2FitResult,
            minimumXLayers, minimumYLayers);

        // 失败结果中的标量和 TVector3 均为 -999，不参与任何角度或直方图计算
        //由于TVector3(-999,-999,-999).Theta会返回固定值，所以要用FitSuccess终止计算
        if (bdc1FitSuccess) {
            const Double_t myThetaX = TMath::ATan(BDC1FitResult.a);
            const Double_t myThetaY = TMath::ATan(BDC1FitResult.c);
            if (TMath::Finite(BDC1_Anaroot_X) && BDC1_Anaroot_X != -999.0) {
                hBDC1XComparison->Fill(BDC1_Anaroot_X, BDC1FitResult.b);
                hBDC1DeltaX->Fill(BDC1FitResult.b - BDC1_Anaroot_X);
            }
            if (TMath::Finite(BDC1_Anaroot_Y) && BDC1_Anaroot_Y != -999.0) {
                hBDC1YComparison->Fill(BDC1_Anaroot_Y, BDC1FitResult.d);
                hBDC1DeltaY->Fill(BDC1FitResult.d - BDC1_Anaroot_Y);
            }
            if (TMath::Finite(BDC1_Anaroot_ThetaX) && BDC1_Anaroot_ThetaX != -999.0) {
                hBDC1ThetaXComparison->Fill(BDC1_Anaroot_ThetaX, myThetaX);
                hBDC1DeltaThetaX->Fill(myThetaX - BDC1_Anaroot_ThetaX);
            }
            if (TMath::Finite(BDC1_Anaroot_ThetaY) && BDC1_Anaroot_ThetaY != -999.0) {
                hBDC1ThetaYComparison->Fill(BDC1_Anaroot_ThetaY, myThetaY);
                hBDC1DeltaThetaY->Fill(myThetaY - BDC1_Anaroot_ThetaY);
            }
        }

        if (bdc2FitSuccess) {
            const Double_t myThetaX = TMath::ATan(BDC2FitResult.a);
            const Double_t myThetaY = TMath::ATan(BDC2FitResult.c);
            if (TMath::Finite(BDC2_Anaroot_X) && BDC2_Anaroot_X != -999.0) {
                hBDC2XComparison->Fill(BDC2_Anaroot_X, BDC2FitResult.b);
                hBDC2DeltaX->Fill(BDC2FitResult.b - BDC2_Anaroot_X);
            }
            if (TMath::Finite(BDC2_Anaroot_Y) && BDC2_Anaroot_Y != -999.0) {
                hBDC2YComparison->Fill(BDC2_Anaroot_Y, BDC2FitResult.d);
                hBDC2DeltaY->Fill(BDC2FitResult.d - BDC2_Anaroot_Y);
            }
            if (TMath::Finite(BDC2_Anaroot_ThetaX) && BDC2_Anaroot_ThetaX != -999.0) {
                hBDC2ThetaXComparison->Fill(BDC2_Anaroot_ThetaX, myThetaX);
                hBDC2DeltaThetaX->Fill(myThetaX - BDC2_Anaroot_ThetaX);
            }
            if (TMath::Finite(BDC2_Anaroot_ThetaY) && BDC2_Anaroot_ThetaY != -999.0) {
                hBDC2ThetaYComparison->Fill(BDC2_Anaroot_ThetaY, myThetaY);
                hBDC2DeltaThetaY->Fill(myThetaY - BDC2_Anaroot_ThetaY);
            }
        }

        outputTree->Fill();
    }

    outputFile->cd();
    outputFile->Write();
    outputFile->Close();
    inputFile->Close();

    std::cout << "Output ROOT file: " << outputFilename << std::endl;
}
