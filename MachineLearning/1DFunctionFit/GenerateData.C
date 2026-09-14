/*
功能：在 x=[-5,5]、步长 0.02 的网格上生成二次函数含噪数据，
      Data.root 保存 x、y、noise，Data.pdf 绘制数据、1 sigma 误差棒和理论曲线。
方法：按等间隔网格计算 y=x^2+x+2+noise，其中 noise 服从 sigma=1 的高斯分布，
      并使用 ROOT TTree 保存数据、TGraphErrors 绘图。
注意事项：网格包含 -5 和 5，共 501 个点；TRandom3(0) 使用 ROOT 的自动种子。
*/

#include "TAxis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TRandom3.h"
#include "TStyle.h"
#include "TTree.h"

void GenerateData() {
  const Double_t xMin = -5.0;
  const Double_t xStep = 0.02;
  const Double_t sigma = 1.0;
  const Int_t pointCount = 501;

  Double_t graphX[pointCount], graphY[pointCount], graphYError[pointCount];
  Double_t x = 0.0, y = 0.0, noise = 0.0;

  TFile *outputFile = new TFile("Data.root", "RECREATE");
  TTree *tree = new TTree("tree", "Quadratic data with Gaussian noise");
  tree->Branch("x", &x, "x/D");
  tree->Branch("y", &y, "y/D");
  tree->Branch("noise", &noise, "noise/D");

  TRandom3 *random = new TRandom3(0);

  for (Int_t i = 0; i < pointCount; ++i) {
    x = xMin + i * xStep;
    noise = random->Gaus(0.0, sigma);
    y = x * x + x + 2.0 + noise;
    tree->Fill();

    graphX[i] = x;
    graphY[i] = y;
    graphYError[i] = sigma; //与graphYError[i] = TMath::Abs(random->Gaus(0.0, sigma)) 有很大区别
    //第一种每一个点的误差棒的长度是固定的sigma，第二种每一个点的误差棒的长度是高斯随机的
    //第一种是正确的，第二种在加权最小二乘拟合中会出现错误
    //第二种表达的意思是恰好落在理论曲线附近的点非常精确；恰好离理论曲线较远的点非常不精确，误差小的点在最小二乘拟合中的权重很大
    //这是不合理的，因为把随机涨落的实现值错误地当成了测量不确定度
  }

  tree->Write();
  outputFile->Close();

  gStyle->SetOptDate(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);

  TGraphErrors *g1 = new TGraphErrors(pointCount, graphX, graphY, 0, graphYError);
  g1->SetTitle(";x;y");
  g1->SetMarkerStyle(20);
  g1->SetMarkerSize(0.8);
  g1->SetMarkerColor(kAzure - 4);
  g1->SetLineColor(kAzure - 4);
  g1->SetMinimum(-5.0);
  g1->SetMaximum(15.0);
  g1->Draw("AP");
  g1->GetXaxis()->SetLimits(-5.0, 5.0);
  g1->GetXaxis()->CenterTitle(kTRUE);
  g1->GetYaxis()->CenterTitle(kTRUE);

  TF1 *f1 = new TF1("f1", "x*x + x + 2", -5.0, 5.0);
  f1->SetLineColor(kRed);
  f1->SetLineWidth(2);
  f1->Draw("L SAME");

  c1->SaveAs("Data.pdf");
}
