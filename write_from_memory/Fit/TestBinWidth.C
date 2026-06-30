#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/1DFunction/My1DFunctions.h"
#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"
#include <iostream>

void TestBinWidth() {
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *h1 = new TH1F("h1", "Ex;Ex(ADC);Counts", 1000, 0, 2000);

  TCanvas *c2 = new TCanvas("c2", "c2", 1000, 800);
  TH1F *h2 = new TH1F("h2", "Ex;Ex(ADC);Counts", 100, 0, 2000);

  TF1 *fgauss1 = new TF1("fgauss1", My1D::Gaussfunction, 700, 1200, 3);
  fgauss1->SetParameters(450, 950, 70);
  fgauss1->SetLineColor(kRed);
  fgauss1->SetLineWidth(5);

  TF1 *fgauss2 = new TF1("fgauss2", My1D::Gaussfunction, 700, 1200, 3);
  fgauss2->SetParameters(5000, 950, 70);
  fgauss2->SetLineColor(kRed);
  fgauss2->SetLineWidth(5);

  Double_t Ex;

  TRandom r(0);

  for (Int_t evt = 0; evt < 50000; evt++) {
    Ex = r.Gaus(950, 70);
    h1->Fill(Ex);
    h2->Fill(Ex);
  }

  c1->cd();
  h1->Draw();
  h1->Fit(fgauss1, "R");

  c2->cd();
  h2->Draw();
  h2->Fit(fgauss2, "R");

  // 拟合
  Double_t mu1 = fgauss1->GetParameter(1);
  Double_t sigma1 = fgauss1->GetParameter(2);
  Double_t mu2 = fgauss2->GetParameter(1);
  Double_t sigma2 = fgauss2->GetParameter(2);

  Double_t x1 = mu1 - 5 * sigma1;
  Double_t x2 = mu1 + 5 * sigma1;

  Double_t x3 = mu2 - 5 * sigma2;
  Double_t x4 = mu2 + 5 * sigma2;

  Double_t BinWidth1 = h1->GetXaxis()->GetBinWidth(1);

  Double_t IntegarlValue1 = fgauss1->Integral(x1, x2);
  std::cout << "Raw Integral fgauss1= " << IntegarlValue1 << std::endl;
  std::cout << "f1 binwidth= " << BinWidth1 << std::endl;
  std::cout << "Integral fgauss1= " << IntegarlValue1 / BinWidth1 << std::endl;

  Double_t BinWidth2 = h2->GetXaxis()->GetBinWidth(1);

  Double_t IntegarlValue2 = fgauss2->Integral(x3, x4);
  std::cout<<std::endl;
  std::cout << "Raw Integral fgauss2= " << IntegarlValue2 << std::endl;
  std::cout << "f2 binwidth= " << BinWidth2 << std::endl;
  std::cout << "Integral fgauss1= " << IntegarlValue2 / BinWidth2 << std::endl;
}
