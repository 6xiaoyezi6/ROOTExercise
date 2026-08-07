#include "GetterRDC.h"

#include <TCanvas.h>
#include <TH1.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>

void SetRDCHistAxisTitle(TH1 *hist, const char *xTitle,
                         const char *xUnit) {
  Double_t binWidth = hist->GetXaxis()->GetBinWidth(1);

  hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void GetterRDC() {
  gStyle->SetOptStat(111111111);

  GetterRDCReader *reader = new GetterRDCReader("run0291_ESPRI.root");

  gROOT->cd();

  TH1D *hLeftRdcABySide = new TH1D(
      "hLeftRdcABySide", "GetrdcA(0): Left RDC A", 200, -0.2, 0.2);
  TH1D *hRightRdcABySide = new TH1D(
      "hRightRdcABySide", "GetrdcA(1): Right RDC A", 200, -0.2, 0.2);
  TH1D *hLeftRdcAByName = new TH1D(
      "hLeftRdcAByName", "GetLeftrdcA(): Left RDC A", 200, -0.2, 0.2);
  TH1D *hRightRdcAByName = new TH1D(
      "hRightRdcAByName", "GetRightrdcA(): Right RDC A", 200, -0.2, 0.2);

  SetRDCHistAxisTitle(hLeftRdcABySide, "Left RDC A", "dx/dz");
  SetRDCHistAxisTitle(hRightRdcABySide, "Right RDC A", "dx/dz");
  SetRDCHistAxisTitle(hLeftRdcAByName, "Left RDC A", "dx/dz");
  SetRDCHistAxisTitle(hRightRdcAByName, "Right RDC A", "dx/dz");

  const Double_t kInvalidValue = -999.0;
  Long64_t numberOfEntries = reader->GetEntries();

  for (Long64_t entry = 0; entry < numberOfEntries; entry++) {
    reader->GetEntry(entry);

    Double_t leftRdcABySide = reader->GetrdcA(0);
    Double_t rightRdcABySide = reader->GetrdcA(1);
    Double_t leftRdcAByName = reader->GetLeftrdcA();
    Double_t rightRdcAByName = reader->GetRightrdcA();

    if (leftRdcABySide != kInvalidValue) {
      hLeftRdcABySide->Fill(leftRdcABySide);
    }
    if (rightRdcABySide != kInvalidValue) {
      hRightRdcABySide->Fill(rightRdcABySide);
    }
    if (leftRdcAByName != kInvalidValue) {
      hLeftRdcAByName->Fill(leftRdcAByName);
    }
    if (rightRdcAByName != kInvalidValue) {
      hRightRdcAByName->Fill(rightRdcAByName);
    }
  }

  TCanvas *canvas = new TCanvas("canvasRDC", "RDC A Getter Comparison",
                                1200, 900);
  canvas->Divide(2, 2);

  canvas->cd(1);
  hLeftRdcABySide->Draw();

  canvas->cd(2);
  hRightRdcABySide->Draw();

  canvas->cd(3);
  hLeftRdcAByName->Draw();

  canvas->cd(4);
  hRightRdcAByName->Draw();

  canvas->Modified();
  canvas->Update();

  delete reader;
}