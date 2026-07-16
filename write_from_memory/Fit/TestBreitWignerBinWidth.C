#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/1DFunction/My1DFunctions.h"

#include "RtypesCore.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TString.h"
#include "TStyle.h"

#include <iostream>

// 自动设置 TH1F 的横轴和纵轴标题
void SetBreitWignerHistAxisTitle(TH1F *h, const char *xTitle,
                                 const char *xUnit) {
  Double_t binWidth = h->GetXaxis()->GetBinWidth(1);

  h->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  h->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void TestBreitWignerBinWidth() {
  gStyle->SetOptStat(111111111);

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *h1 = new TH1F("h1", "Breit-Wigner", 1000, 0, 2000);

  TCanvas *c2 = new TCanvas("c2", "c2", 1000, 800);
  TH1F *h2 = new TH1F("h2", "Breit-Wigner", 100, 0, 2000);

  // 自动设置坐标轴标题
  SetBreitWignerHistAxisTitle(h1, "Ex", "ADC");
  SetBreitWignerHistAxisTitle(h2, "Ex", "ADC");

  // BreitWigner 的参数顺序：A、mean、gamma
  TF1 *fBreitWigner1 =
      new TF1("fBreitWigner1", My1D::BreitWigner, 700, 1200, 3);
  fBreitWigner1->SetParameters(100000, 950, 70);
  fBreitWigner1->SetParNames("A", "mean", "gamma");
  fBreitWigner1->SetLineColor(kRed);
  fBreitWigner1->SetLineWidth(5);

  TF1 *fBreitWigner2 =
      new TF1("fBreitWigner2", My1D::BreitWigner, 700, 1200, 3);
  fBreitWigner2->SetParameters(1000000, 950, 70);
  fBreitWigner2->SetParNames("A", "mean", "gamma");
  fBreitWigner2->SetLineColor(kRed);
  fBreitWigner2->SetLineWidth(5);

  Double_t Ex;
  TRandom r(0);

  for (Int_t evt = 0; evt < 50000; evt++) {
    Ex = r.BreitWigner(950, 70);
    h1->Fill(Ex);
    h2->Fill(Ex);
  }

  c1->cd();
  h1->Draw();
  h1->Fit(fBreitWigner1, "R");

  c2->cd();
  h2->Draw();
  h2->Fit(fBreitWigner2, "R");

  // 读取拟合参数
  Double_t mean1 = fBreitWigner1->GetParameter(1);
  Double_t gamma1 = fBreitWigner1->GetParameter(2);

  Double_t mean2 = fBreitWigner2->GetParameter(1);
  Double_t gamma2 = fBreitWigner2->GetParameter(2);

  // 积分区间：mean ± 5 gamma
  Double_t x1 = mean1 - 5 * gamma1;
  Double_t x2 = mean1 + 5 * gamma1;

  Double_t x3 = mean2 - 5 * gamma2;
  Double_t x4 = mean2 + 5 * gamma2;

  // h1 的积分
  Double_t BinWidth1 = h1->GetXaxis()->GetBinWidth(1);
  Double_t IntegralValue1 = fBreitWigner1->Integral(x1, x2);

  std::cout << "Raw Integral fBreitWigner1 = " << IntegralValue1
            << std::endl;
  std::cout << "fBreitWigner1 bin width = " << BinWidth1 << std::endl;
  std::cout << "Integral fBreitWigner1 / bin width = "
            << IntegralValue1 / BinWidth1 << std::endl;

  // h2 的积分
  Double_t BinWidth2 = h2->GetXaxis()->GetBinWidth(1);
  Double_t IntegralValue2 = fBreitWigner2->Integral(x3, x4);

  std::cout << std::endl;
  std::cout << "Raw Integral fBreitWigner2 = " << IntegralValue2
            << std::endl;
  std::cout << "fBreitWigner2 bin width = " << BinWidth2 << std::endl;
  std::cout << "Integral fBreitWigner2 / bin width = "
            << IntegralValue2 / BinWidth2 << std::endl;
}
