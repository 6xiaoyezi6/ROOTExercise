#include <iostream>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TStyle.h"
#include "TTree.h"

#include "TClusterGAGGData.h"

#include "/Users/yemingxin/nptool/Projects/myTOGAXSI_ppalpha/TOGAXSIReconstrution/TOGAXSIReconstrution.h"
#include "/Users/yemingxin/nptool/Projects/myTOGAXSI_ppalpha/TOGAXSIReconstrution/TOGAXSIReconstrution.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.cc"

void SetHistAxisTitle(TH1* h, const char* xTitle, const char* xUnit) {
  Double_t binWidth = h->GetXaxis()->GetBinWidth(1);
  h->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  h->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void PID() {
  Timer timer;
  gStyle->SetOptStat(111111111);

  constexpr Double_t alpha = 0.7;
  constexpr Double_t beta = 0.005;

  TFile* inputFile = TFile::Open("PID.root", "READ");
  TTree* inputTree = (TTree*)inputFile->Get("SimulatedTree");

  TClusterGAGGData* cg = nullptr;
  inputTree->SetBranchAddress("ClusterGAGG", &cg);

  Double_t deltaE = 0.0;
  Double_t E = 0.0;
  Double_t Ef = 0.0;

  TH2D* hPID = new TH2D(
      "hPID",
      "deltaE-E Particle Identification;E(MeV);#Delta E (MeV)",
      500, 0.0, 800.0,
      500, 0.0, 600.0);
  hPID->SetDirectory(nullptr);

  TH1D* hEf = new TH1D("hEf", "E_{f} Particle Identification", 1000, 0.0, 900.0);
  hEf->SetDirectory(nullptr);
  SetHistAxisTitle(hEf, "E_{f}", "MeV");

  TH2D* hEfE = new TH2D(
      "hEfE",
      "E_{f}-E Particle Identification;E (MeV);E_{f} (MeV)",
      500, 0.0, 800.0,
      500, 0.0, 900.0);
  hEfE->SetDirectory(nullptr);

  TFile* outputFile = TFile::Open("PID_deltaE_E.root", "RECREATE");
  TTree* outputTree = new TTree("PIDTree", "ClusterGAGG #DeltaE-E data");
  outputTree->Branch("deltaE", &deltaE, "deltaE/D");
  outputTree->Branch("E", &E, "E/D");
  outputTree->Branch("Ef", &Ef, "Ef/D");

  Long64_t coincidenceEntries = 0;
  const Long64_t entries = inputTree->GetEntries();
  for(Long64_t entry = 0; entry < entries; entry++) {
    inputTree->GetEntry(entry);

    E = SumClusterGAGGEnergy(cg);
    const Double_t deltaERight = SumDeltaEGAGGEnergy(cg, "Right");
    const Double_t deltaELeft = SumDeltaEGAGGEnergy(cg, "Left");

    if(deltaERight > 0.0 && E > 0.0) {
      deltaE = deltaERight;
      //变换公式E_{f}=\sqrt{\Delta E \cdot E+\alpha \Delta E^{2}}+\beta E
      Ef = TMath::Sqrt(deltaE * E + alpha * deltaE * deltaE) + beta * E;
      hPID->Fill(E, deltaE);
      hEf->Fill(Ef);
      hEfE->Fill(E, Ef);
      outputTree->Fill();
      coincidenceEntries++;
    }
    if(deltaELeft > 0.0 && E > 0.0) {
      deltaE = deltaELeft;
      Ef = TMath::Sqrt(deltaE * E + alpha * deltaE * deltaE) + beta * E;
      hPID->Fill(E, deltaE);
      hEf->Fill(Ef);
      hEfE->Fill(E, Ef);
      outputTree->Fill();
      coincidenceEntries++;
    }
  }

  outputFile->cd();
  outputTree->Write();
  hEf->Write();
  hEfE->Write();
  outputFile->Close();
  inputFile->Close();

  TCanvas* canvas = new TCanvas("canvasPID", "#DeltaE-E PID", 1000, 800);
  canvas->SetRightMargin(0.14);
  canvas->SetLogz();
  hPID->Draw("COLZ");
  canvas->Update();
  canvas->SaveAs("PID_deltaE_E.pdf");

  TCanvas* canvasEfE = new TCanvas("canvasEfE", "E_{f}-E PID", 1000, 800);
  canvasEfE->SetRightMargin(0.14);
  canvasEfE->SetLogz();
  hEfE->Draw("COLZ");
  canvasEfE->Update();
  canvasEfE->SaveAs("PID_Ef_E.pdf");

  TCanvas* canvasEf = new TCanvas("canvasEf", "E_{f} PID", 1000, 800);
  canvasEf->SetLogy();
  hEf->SetLineWidth(2);
  hEf->SetLineColor(kBlue + 2);
  hEf->SetFillColor(kAzure - 9);
  hEf->Draw("HIST");
  canvasEf->Update();
  canvasEf->SaveAs("PID_Ef.pdf");

}
