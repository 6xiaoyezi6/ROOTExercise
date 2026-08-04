#include "GetterBDCRDC.h"

#include <TCanvas.h>
#include <TH1.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>

void SetBDCRDCHistAxisTitle(TH1 *hist, const char *xTitle,
                            const char *xUnit) {
  Double_t binWidth = hist->GetXaxis()->GetBinWidth(1);

  hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void GetterBDCRDC() {
  gStyle->SetOptStat(111111111);

  GetterBDCRDCReader *reader = new GetterBDCRDCReader("run0291_BDC.root", "run0291_ESPRI.root");

  gROOT->cd();

  TH1D *hBDC1ThetaX = new TH1D(
      "hCombinedBDC1ThetaX", "GetBDC1ThetaX(): BDC1 ThetaX",
      200, -0.2, 0.2);
  TH1D *hLeftRdcABySide = new TH1D(
      "hCombinedLeftRdcABySide", "GetrdcA(0): Left RDC A",
      200, -0.2, 0.2);
  TH1D *hRightRdcABySide = new TH1D(
      "hCombinedRightRdcABySide", "GetrdcA(1): Right RDC A",
      200, -0.2, 0.2);
  TH1D *hLeftRdcAByName = new TH1D(
      "hCombinedLeftRdcAByName", "GetLeftrdcA(): Left RDC A",
      200, -0.2, 0.2);
  TH1D *hRightRdcAByName = new TH1D(
      "hCombinedRightRdcAByName", "GetRightrdcA(): Right RDC A",
      200, -0.2, 0.2);

  SetBDCRDCHistAxisTitle(hBDC1ThetaX, "BDC1 ThetaX", "rad");
  SetBDCRDCHistAxisTitle(hLeftRdcABySide, "Left RDC A", "dx/dz");
  SetBDCRDCHistAxisTitle(hRightRdcABySide, "Right RDC A", "dx/dz");
  SetBDCRDCHistAxisTitle(hLeftRdcAByName, "Left RDC A", "dx/dz");
  SetBDCRDCHistAxisTitle(hRightRdcAByName, "Right RDC A", "dx/dz");

  const Double_t kInvalidValue = -999.0;
  Long64_t numberOfEntries = reader->GetEntries();

  for (Long64_t entry = 0; entry < numberOfEntries; entry++) {
    reader->GetEntry(entry);

    Double_t bdc1ThetaX = reader->GetBDC1ThetaX();
    Double_t leftRdcABySide = reader->GetrdcA(0);
    Double_t rightRdcABySide = reader->GetrdcA(1);
    Double_t leftRdcAByName = reader->GetLeftrdcA();
    Double_t rightRdcAByName = reader->GetRightrdcA();

    if (bdc1ThetaX != kInvalidValue) hBDC1ThetaX->Fill(bdc1ThetaX);
    
    if (leftRdcABySide != kInvalidValue) hLeftRdcABySide->Fill(leftRdcABySide);
  
    if (rightRdcABySide != kInvalidValue) hRightRdcABySide->Fill(rightRdcABySide);
    
    if (leftRdcAByName != kInvalidValue) hLeftRdcAByName->Fill(leftRdcAByName);
    
    if (rightRdcAByName != kInvalidValue) hRightRdcAByName->Fill(rightRdcAByName);
    
  }

  TCanvas *canvas = new TCanvas("canvasBDCRDC", "BDC and RDC Getter Inheritance", 1500, 900);
  canvas->Divide(3, 2);

  canvas->cd(1);
  hBDC1ThetaX->Draw();

  canvas->cd(2);
  hLeftRdcABySide->Draw();

  canvas->cd(3);
  hRightRdcABySide->Draw();

  canvas->cd(4);
  hLeftRdcAByName->Draw();

  canvas->cd(5);
  hRightRdcAByName->Draw();

  canvas->Modified();
  canvas->Update();

  delete reader;
}
