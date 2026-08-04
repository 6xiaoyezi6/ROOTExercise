#include "GetterBDC.h"

#include <TCanvas.h>
#include <TH1.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>

void SetHistAxisTitle(TH1 *hist, const char *xTitle, const char *xUnit) {
  Double_t binWidth = hist->GetXaxis()->GetBinWidth(1);

  hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void GetterBDC() {
  gStyle->SetOptStat(111111111);

  //GetterBDCReader是类名，reader是用户起的别名
  GetterBDCReader *reader = new GetterBDCReader("run0291_BDC.root");

  gROOT->cd();

  TH1D *hBDC1ThetaX =new TH1D("hBDC1ThetaX", "BDC1 ThetaX", 200, -0.2, 0.2);
  SetHistAxisTitle(hBDC1ThetaX, "BDC1 ThetaX", "rad");

  const Double_t invalidValue = -999.0;
  Long64_t numberOfEntries = reader->GetEntries();

  for (Long64_t entry = 0; entry < numberOfEntries; entry++) {
    reader->GetEntry(entry);

    Double_t bdc1ThetaX = reader->GetBDC1ThetaX();
    if (bdc1ThetaX != invalidValue) {
      hBDC1ThetaX->Fill(bdc1ThetaX);
    }
  }

  TCanvas *canvas = new TCanvas("canvas", "BDC1 ThetaX", 900, 700);
  hBDC1ThetaX->Draw();
  canvas->Modified();
  canvas->Update();

  delete reader;
}
