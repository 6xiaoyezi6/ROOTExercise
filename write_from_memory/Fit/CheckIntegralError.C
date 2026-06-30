#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/1DFunction/My1DFunctions.h"

#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TH1F.h"
#include "TMath.h"
#include "TMatrixDSym.h"
#include "TRandom.h"
#include "TString.h"
#include "TTree.h"
#include "TVector3.h"

#include <iostream>
#include <iterator>
#include <ostream>

// 自动设置 TH1F 的横轴和纵轴标题
void SetHistAxisTitle(TH1F *h, const char *xTitle, const char *xUnit) {
  Double_t binWidth = h->GetXaxis()->GetBinWidth(1);

  h->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  h->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void CheckIntegralError() {
  TFile *f = TFile::Open("/Users/yemingxin/RIKEN_Inelastic_Be/ppBe10/"
                         "cross-section/output/generate.root",
                         "READ");
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *hFile = (TH1F *)f->Get("hExBe10_63_64");
  TH1F *h1 = (TH1F *)hFile->Clone("h2");

  TF1 *fgauss1 = new TF1("fgauss1", My1D::Gaussfunction, -1.6, 2, 3);
  fgauss1->SetParameters(14.25, 0, 0.7);
  fgauss1->SetLineColor(kRed);
  fgauss1->SetLineWidth(5);

  h1->Draw();
  h1->Fit(fgauss1, "R");

  TFitResultPtr fitResult = h1->Fit(fgauss1, "SQRB");

  //直接取协方差矩阵元
  Double_t CovASigma = fitResult->CovMatrix(0, 2);
  // 打印完整协方差矩阵和相关系数
  fitResult->Print("V");


  // 读取拟合参数
  Double_t A1 = fgauss1->GetParameter(0);
  Double_t mu1 = fgauss1->GetParameter(1);
  Double_t sigma1 = fgauss1->GetParameter(2);

  // 积分区间：mu ± 5 sigma
  Double_t x1 = mu1 - 5 * sigma1;
  Double_t x2 = mu1 + 5 * sigma1;

  // h1 的积分
  Double_t BinWidth1 = h1->GetXaxis()->GetBinWidth(1);
  Double_t IntegralValue1 = fgauss1->Integral(x1, x2);

  std::cout << "Raw Integral fgauss1 = " << IntegralValue1 << std::endl;
  std::cout << "fgauss1 bin width = " << BinWidth1 << std::endl;
  std::cout << "Integral fgauss1 / bin width = " << IntegralValue1 / BinWidth1
            << std::endl;

  // 计数误差（积分误差）分析
  Double_t deltaA = fgauss1->GetParError(0);
  Double_t deltaSigma = fgauss1->GetParError(2);
  std::cout << std::endl;
  std::cout << "deltaA = " << deltaA << std::endl;
  std::cout << "deltaSigma = " << deltaSigma << std::endl;
  std::cout << "CovASigma = " << CovASigma << std::endl;

  const Double_t K =(TMath::Erf(5 / (TMath::Power(2, 0.5))) * TMath::Sqrt(2 * TMath::Pi()))/BinWidth1;
  Double_t deltaCounts_square = deltaA*deltaA * TMath::Power(K * sigma1, 2) +
                         deltaSigma*deltaSigma * TMath::Power(K * A1, 2) +
                         2 * K * K * sigma1 * A1 * CovASigma;
  Double_t deltaCounts = TMath::Sqrt(deltaCounts_square);

  std::cout << "deltaCounts = " << deltaCounts << std::endl; 
}
