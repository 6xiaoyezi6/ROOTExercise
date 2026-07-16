#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/1DFunction/My1DFunctions.h"

#include "RtypesCore.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TRandom.h"
#include "TString.h"
#include "TStyle.h"

#include <iostream>

// 自动设置一维直方图的横轴和纵轴标题
void SetGaussBreitWignerAxisTitle(TH1F *h, const char *xTitle,
                                  const char *xUnit) {
  Double_t binWidth = h->GetXaxis()->GetBinWidth(1);

  h->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  h->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void GaussBreitWigner() {
  gStyle->SetOptStat(111111111);
  gStyle->SetOptFit(1111);

  TCanvas *c1 = new TCanvas("c1", "Gauss and Breit-Wigner", 1200, 850);

  // 200 bins、范围 0--2000，因此 bin width 为 10 ADC
  TH1F *hGauss = new TH1F("hGauss", "Gauss and Breit-Wigner", 200, 0, 2000);
  TH1F *hBreitWigner =
      new TH1F("hBreitWigner", "Gauss and Breit-Wigner", 200, 0, 2000);

  SetGaussBreitWignerAxisTitle(hGauss, "Ex", "ADC");
  SetGaussBreitWignerAxisTitle(hBreitWigner, "Ex", "ADC");

  TRandom r(0);
  Double_t gaussValue;
  Double_t breitWignerValue;

  for (Int_t evt = 0; evt < 50000; evt++) {
    gaussValue = r.Gaus(950, 70);
    breitWignerValue = r.BreitWigner(950, 70);

    hGauss->Fill(gaussValue);
    hBreitWigner->Fill(breitWignerValue);
  }

  // 高斯拟合函数参数顺序：A、mu、sigma
  TF1 *fgs = new TF1("fgs", My1D::Gaussfunction, 700, 1200, 3);
  fgs->SetParameters(5500, 950, 70);
  fgs->SetParNames("A", "mu", "sigma");
  fgs->SetLineColor(kBlue + 1);
  fgs->SetLineWidth(2);

  // Breit-Wigner 拟合函数参数顺序：A、mean、gamma
  TF1 *fbw = new TF1("fbw", My1D::BreitWigner, 700, 1200, 3);
  fbw->SetParameters(1000000, 950, 70);
  fbw->SetParNames("A", "mean", "gamma");
  fbw->SetLineColor(kRed + 1);
  fbw->SetLineWidth(2);

  hGauss->Fit(fgs, "R0");
  hBreitWigner->Fit(fbw, "R0");

  Double_t maximum = hGauss->GetMaximum();
  if (hBreitWigner->GetMaximum() > maximum) {
    maximum = hBreitWigner->GetMaximum();
  }
  hGauss->SetMaximum(1.25 * maximum);

  c1->cd();
  hGauss->Draw();
  hBreitWigner->Draw("SAMES");
  c1->Update();

  // 分开摆放两个直方图的统计框
  TPaveStats *statsGauss = (TPaveStats *)hGauss->FindObject("stats");
  TPaveStats *statsBreitWigner =
      (TPaveStats *)hBreitWigner->FindObject("stats");

  if (statsGauss) {
    statsGauss->SetTextColor(kBlue + 1);
    statsGauss->SetY1NDC(0.62);
    statsGauss->SetY2NDC(0.88);
  }

  if (statsBreitWigner) {
    statsBreitWigner->SetTextColor(kRed + 1);
    statsBreitWigner->SetY1NDC(0.34);
    statsBreitWigner->SetY2NDC(0.60);
  }

  // 高斯积分区间：mu ± 3 sigma
  Double_t mu = fgs->GetParameter(1);
  Double_t sigma = fgs->GetParameter(2);
  Double_t x1 = mu - 3.0 * sigma;
  Double_t x2 = mu + 3.0 * sigma;

  TF1 *hGaussFill = (TF1 *)fgs->DrawCopy("SAME");
  hGaussFill->SetRange(x1, x2);
  hGaussFill->SetFillColor(kBlue + 1);
  hGaussFill->SetFillStyle(3001);
  hGaussFill->SetLineColor(kBlue + 2);
  hGaussFill->SetBit(kCanDelete);
  hGaussFill->Draw("F SAME");

  // Breit-Wigner 积分区间：mean ± 3 gamma
  Double_t mean = fbw->GetParameter(1);
  Double_t gamma = fbw->GetParameter(2);
  Double_t x3 = mean - 3.0 * gamma;
  Double_t x4 = mean + 3.0 * gamma;

  TF1 *hBreitWignerFill = (TF1 *)fbw->DrawCopy("SAME");
  hBreitWignerFill->SetRange(x3, x4);
  hBreitWignerFill->SetFillColor(kGreen + 1);
  hBreitWignerFill->SetFillStyle(3001);
  hBreitWignerFill->SetLineColor(kGreen + 2);
  hBreitWignerFill->SetBit(kCanDelete);
  hBreitWignerFill->Draw("F SAME");

  // 再画一次拟合曲线和直方图，使其显示在填色区域上方
  fgs->Draw("SAME");
  fbw->Draw("SAME");
  hGauss->Draw("SAME");
  hBreitWigner->Draw("SAME");

  TLegend *legend = new TLegend(0.14, 0.72, 0.43, 0.88);
  legend->AddEntry(hGauss, "Gaussian histogram", "l");
  legend->AddEntry(fgs, "Gaussian fit and integral", "lf");
  legend->AddEntry(hBreitWigner, "Breit-Wigner histogram", "l");
  legend->AddEntry(fbw, "Breit-Wigner fit and integral", "lf");
  legend->Draw();

  Double_t binWidth = hGauss->GetXaxis()->GetBinWidth(1);
  Double_t gaussIntegral = fgs->Integral(x1, x2);
  Double_t breitWignerIntegral = fbw->Integral(x3, x4);

  std::cout << "Gauss integral / bin width = "
            << gaussIntegral / binWidth << std::endl;
  std::cout << "Breit-Wigner integral / bin width = "
            << breitWignerIntegral / binWidth << std::endl;

  c1->Modified();
  c1->Update();
  c1->SaveAs("GaussBW.pdf");
}
