#include "RtypesCore.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TStyle.h"
#include "TSpectrum.h"

#include <algorithm>
#include <iostream>

// 使用TSpectrum自动进行本底扣除和峰寻找，处理的是Array.C生成的数据
void AutoSearchPeak() {
  gStyle->SetOptStat(111111111);

  TFile *f = TFile::Open("Array.root", "READ");
  TH1F *h = (TH1F *)f->Get("h");
  h->SetDirectory(0);//让柱状图不随文件而关闭

  const Int_t nMaximumPeaks = 10;
  TSpectrum*s = new TSpectrum(nMaximumPeaks);

  TH1F *hBackground = (TH1F *)s->Background(h, 50, "");//Background(输入直方图, 迭代次数, 选项)，迭代次数越大估计的本底通常越平滑，越宽的峰结构会被当作峰而不是本底
  hBackground->SetDirectory(0);
  TH1F *hPeaks = (TH1F *)h->Clone("hPeaks");
  hPeaks->SetDirectory(0);
  hPeaks->SetTitle("Background-subtracted spectrum;E");
  hPeaks->GetYaxis()->SetTitle(Form("Counts / %.3g", hPeaks->GetBinWidth(1)));
  hPeaks->Add(hPeaks, hBackground, 1, -1);//本底扣除hPeaks = 1 × 原始能谱 - 1 × 本底谱

  TCanvas *canvas = new TCanvas("canvas", "TSpectrum peak search", 900, 700);
  canvas->Divide(1, 2);

  canvas->cd(1);
  h->Draw();
  hBackground->SetLineColor(kRed);
  hBackground->Draw("same");

  canvas->cd(2);
  hPeaks->Draw("hist");
  Int_t nFound = s->Search(hPeaks, 3, "hist", 0.01);//Search(直方图, 寻找的峰宽度sigma, 选项, threshold)
  //减小threshold会增加候选峰的数目

  Double_t peakPositions[nMaximumPeaks];
  Double_t *xPeaks = s->GetPositionX();
  for (Int_t i = 0; i < nFound; i++) {
    peakPositions[i] = xPeaks[i];
  }
  std::sort(peakPositions, peakPositions + nFound);

  //如果需要更精确的峰中心，通常以 TSpectrum 的结果作为初值，再在峰附近进行高斯拟合，而不是直接把 GetPositionX() 当作最终峰位
  std::cout << "找到 " << nFound << " 个峰：" << std::endl;
  for (Int_t i = 0; i < nFound; i++) {
    std::cout << "峰 " << i + 1 << " 的位置：" << peakPositions[i] << std::endl;
  }


  f->Close();
}
