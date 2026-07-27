#include <limits>

#include "RtypesCore.h"
#include "TFile.h"
#include "TH1D.h"
#include "TRandom.h"
#include "TString.h"
#include "TTree.h"

// 测试填写NAN和-999的区别
//NaN 被 ROOT 放进 overflow bin

void SetHistAxisTitle(TH1D *hist, const char *xTitle, const char *xUnit) {
  Double_t binWidth = hist->GetXaxis()->GetBinWidth(1);

  hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void TestNAN() {
  TFile *fout = TFile::Open("TestNAN.root", "RECREATE");
  TTree *tree = new TTree("tree", "tree with NAN and -999");

  Int_t EventID;
  Double_t Ex_NAN;
  Double_t Ex_minus999;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("Ex_NAN", &Ex_NAN, "Ex_NAN/D");
  tree->Branch("Ex_minus999", &Ex_minus999, "Ex_minus999/D");

  TH1D *hExNAN = new TH1D("hEx_NAN", "Excitation energy with NAN", 1000, 0, 1000);
  TH1D *hExMinus999 = new TH1D("hEx_minus999", "Excitation energy with -999", 1000, 0, 1000);
  SetHistAxisTitle(hExNAN, "Ex", "keV");
  SetHistAxisTitle(hExMinus999, "Ex", "keV");

  TRandom r(0);

  for (Int_t i = 0; i < 50000; ++i) {
    EventID = i;

    if (i < 25000) {
      Ex_NAN = r.Gaus(500, 70);
      Ex_minus999 = r.Gaus(500, 70);
    } else {
      Ex_NAN = NAN;
      Ex_minus999 = -999;
    }

    tree->Fill();
    hExNAN->Fill(Ex_NAN);
    hExMinus999->Fill(Ex_minus999);
  }

  fout->cd();
  tree->Write();
  fout->Close();
}
