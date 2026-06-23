#include "Rtypes.h"
#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVector3.h"
#include <iostream>

Double_t GaussFunction(Double_t *x, Double_t *par) {
  Double_t A = par[0];
  Double_t mu = par[1];
  Double_t sigma = par[2];

  return A * TMath::Exp(-0.5 * TMath::Power((x[0] - mu) / sigma, 2));
}

Double_t DoubleGaussFunction(Double_t *x, Double_t *par) {
  return GaussFunction(x, par) + GaussFunction(x, par + 3);
}

void FitGauss_Exercise() {
  TFile *f = TFile::Open("FitGaussData_Exercise.root", "READ");
  TTree *t = (TTree *)f->Get("tree");

  Double_t Ex;

  t->SetBranchAddress("Ex", &Ex);

  // 取树里面的分支进行拟合
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *h1 =
      new TH1F("h1", "Ex From Tree Branch;Ex(ADC);Counts", 1000, 0, 2000);
  Long64_t nentries = t->GetEntriesFast();
  for (Long64_t i = 0; i < nentries; i++) {
    t->GetEntry(i);
    h1->Fill(Ex);
  }

  TF1 *fgauss1 = new TF1("fgauss1", GaussFunction, 700, 1200, 3);
  TF1 *fgauss2 = new TF1("fgauss2", GaussFunction, 1100, 1600, 3);
  TF1 *fgauss = new TF1("fgauss", DoubleGaussFunction, 600, 1800, 6);

  fgauss1->SetParameters(450, 950, 70);
  fgauss1->SetLineColor(kRed);
  fgauss1->SetLineWidth(5);

  fgauss2->SetParameters(200, 1350, 90);
  fgauss2->SetLineColor(kBlue);
  fgauss2->SetLineWidth(5);

  fgauss->SetParameters(450, 950, 70, 200, 1350, 90);
  fgauss->SetLineColor(kGreen);
  fgauss->SetLineColor(kGreen);
  fgauss->SetLineWidth(5);

  h1->Draw();
  h1->Fit(fgauss1, "R");
  h1->Fit(fgauss2, "R+");
  h1->Fit(fgauss, "R+");

  std::cout << "fgauss1 mu = " << fgauss1->GetParameter(1) << std::endl;

  // 取root 文件中的TH1 对象进行拟合
  TCanvas *c2 = new TCanvas("c2", "c2", 1000, 800);
  TH1F *hFile = (TH1F *)f->Get("histograms/h1");
  TH1F *h2 = (TH1F *)hFile->Clone("h2");
  h2->SetTitle("Ex from histograms;Ex(ADC);Counts");

  fgauss1->SetParameters(450, 950, 70);
  fgauss1->SetLineColor(kRed);
  fgauss1->SetLineWidth(5);

  fgauss2->SetParameters(200, 1350, 90);
  fgauss2->SetLineColor(kBlue);
  fgauss2->SetLineWidth(5);

  fgauss->SetParameters(450, 950, 70, 200, 1350, 90);
  fgauss->SetLineColor(kGreen);
  fgauss->SetLineColor(kGreen);
  fgauss->SetLineWidth(5);

  h2->Draw();
  h2->Fit(fgauss1, "R");
  h2->Fit(fgauss2, "R+");
  h2->Fit(fgauss, "R+");
}
