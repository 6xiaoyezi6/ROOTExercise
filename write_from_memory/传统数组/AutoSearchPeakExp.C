#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1F.h"
#include "TStyle.h"
#include "TSpectrum.h"
#include "TTree.h"

#include <algorithm>
#include <iostream>

void SetHistAxisTitleExp(TH1 *h, const char *xTitle, const char *xUnit) {
  Double_t binWidth = h->GetXaxis()->GetBinWidth(1);

  h->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  h->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

// 使用 TSpectrum 对实验数据的 ExWithCut 能谱扣除本底并自动寻峰
void AutoSearchPeakExp() {
  gStyle->SetOptStat(111111111);

  TFile *f = TFile::Open(
      "/Users/yemingxin/RIKEN_Inelastic_Be/ppBe10/BackTracking/"
      "Back2TrackingBDCEspri/Ex.root",
      "READ");
  TTree *tree = (TTree *)f->Get("tree");

  Long64_t nSelected = tree->Draw(
      "ExWithCut>>h(500,-20,30)",
      "ExWithCut!=-999",
      "goff");

  TH1F *h = (TH1F *)gDirectory->Get("h");
  h->SetDirectory(0);
  h->SetTitle("Experimental excitation-energy spectrum");
  SetHistAxisTitleExp(h, "E_{x}", "MeV");


  const Int_t nMaximumPeaks = 10;
  TSpectrum *s = new TSpectrum(nMaximumPeaks);

  TH1F *hBackground =(TH1F *)s->Background(h, 30, "");
  hBackground->SetDirectory(0);
  hBackground->SetLineColor(kRed);
  hBackground->SetLineWidth(2);
  SetHistAxisTitleExp(hBackground, "E_{x}", "MeV");

  TH1F *hPeaks = (TH1F *)h->Clone("hPeaksExp");
  hPeaks->SetDirectory(0);
  hPeaks->SetTitle("Background-subtracted excitation-energy spectrum");
  hPeaks->Add(hBackground, -1.0);
  SetHistAxisTitleExp(hPeaks, "E_{x}", "MeV");

  TCanvas *canvas =
      new TCanvas("canvasExp", "TSpectrum experimental peak search", 900, 700);
  canvas->Divide(1, 2);

  canvas->cd(1);
  h->Draw("hist");
  hBackground->Draw("hist same");

  canvas->cd(2);
  hPeaks->Draw("hist");
  Int_t nFound = s->Search(hPeaks, 2, "hist", 0.01);//Search(直方图, 寻找的峰宽度sigma, 选项, threshold)
  //减小threshold会增加候选峰的数目

  Double_t peakPositions[nMaximumPeaks];
  Double_t *xPeaks = s->GetPositionX();
  for (Int_t i = 0; i < nFound; i++) {
    peakPositions[i] = xPeaks[i];
  }
  std::sort(peakPositions, peakPositions + nFound);

  std::cout << "ExWithCut!=-999 的事例数：" << nSelected << std::endl;
  std::cout << "找到 " << nFound << " 个峰：" << std::endl;
  for (Int_t i = 0; i < nFound; i++) {
    std::cout << "峰 " << i + 1 << " 的位置："
              << peakPositions[i] << " MeV" << std::endl;
  }

  canvas->Update();
  f->Close();
}
