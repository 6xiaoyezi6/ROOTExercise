#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVector3.h"

void FitGauss() {
  TFile *f = new TFile("FitGaussData.root", "read");
  TTree *t = (TTree *)f->Get("tree");

  // 用两种方法1.取树里面的branch进行拟合 2.取root文件中的TH1对象进行拟合

  Float_t Ex;
  t->SetBranchAddress("Ex", &Ex);

  // 读取树中的branch
  TCanvas *c1 = new TCanvas("c1", "Fit Ex from TTree branch", 800, 600);
  TH1F *h1 =
      new TH1F("h1", "Ex from TTree branch;Ex(MeV);counts", 1000, 0, 2000);

  Long64_t nentries = t->GetEntries();
  for (Long64_t i = 0; i < nentries; i++) {
    t->GetEntry(i);
    h1->Fill(Ex);
  }

  TF1 *fgauss1 =
      new TF1("fgauss1", "[0]*exp(-0.5*(pow((x-[1])/[2],2)))", 700, 1200);
  TF1 *fgauss2 =
      new TF1("fgauss2", "[0]*exp(-0.5*(pow((x-[1])/[2],2)))", 1100, 1600);
  TF1 *fgauss = new TF1(
      "fgauss",
      "[0]*exp(-0.5*(pow((x-[1])/[2],2)))+[3]*exp(-0.5*(pow((x-[4])/[5],2)))",
      600, 1800);

  fgauss1->SetParameters(450, 950, 70);
  fgauss1->SetLineColor(kRed);
  fgauss1->SetLineWidth(5);

  fgauss2->SetParameter(0, 200);
  fgauss2->SetParameter(1, 1350);
  fgauss2->SetParameter(2, 90);
  fgauss2->SetLineColor(kBlue);
  fgauss2->SetLineWidth(5);

  fgauss->SetParameters(450, 950, 70, 200, 1350,
                        90); // 注意是SetParameters后面有个s
  fgauss->SetLineColor(kGreen);
  fgauss->SetLineWidth(5);

  h1->Draw();
  h1->Fit(fgauss1, "R");
  h1->Fit(fgauss2, "R+");
  h1->Fit(fgauss, "R+");

  // 读取ROOT文件中的TDirectoryFile中的TH1对象
  TCanvas *c2 = new TCanvas("c2", "Fit Ex from histograms directory", 800, 600);
  TH1F *hFile = (TH1F *)f->Get("histograms/h1");
  TH1F *h2 = (TH1F *)hFile->Clone("h2");
  h2->SetTitle("Ex from histograms/h1;Ex(adc);counts");

  fgauss1->SetParameter(0, 450);
  fgauss1->SetParameter(1, 950);
  fgauss1->SetParameter(2, 70); // 必须要给定拟合初值才可以

  fgauss2->SetParameter(0, 200);
  fgauss2->SetParameter(1, 1350);
  fgauss2->SetParameter(2, 90);
  fgauss->SetParameters(450, 950, 70, 200, 1350,
                        90); // 注意是SetParameters后面有个s
  fgauss->SetLineColor(kGreen);

  h2->Draw();
  h2->Fit(fgauss1, "R");
  h2->Fit(fgauss2, "R+");
  h2->Fit(fgauss, "R+");

  TDatime now;
  TString outdir =
      Form("output/FitGauss_%04d%02d%02d_%02d%02d", now.GetYear(),
           now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute());
  gSystem->mkdir("output", kTRUE);
  gSystem->mkdir(outdir.Data(), kTRUE);

  c2->SaveAs(Form("%s/h2_fit.pdf", outdir.Data()));
}
