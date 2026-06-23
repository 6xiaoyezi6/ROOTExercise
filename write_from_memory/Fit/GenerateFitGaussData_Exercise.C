#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"

void GenerateFitGaussData_Exercise() {
  TFile *fout = TFile::Open("FitGaussData_Exercise.root", "RECREATE");
  TDirectory *histDir = fout->mkdir("histograms");
  TTree *tree = new TTree("tree", "simple event tree", 99,
                          gDirectory); // tree放入默认的gDirectory中

  Int_t evt;
  Double_t Ex;

  tree->Branch("evt", &evt, "evt/I");
  tree->Branch("Ex", &Ex, "Ex/D");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *h1 = new TH1F("h1", "Ex;Ex(ADC);counts", 1000, 0, 2000);

  TRandom r(0);

  for (evt = 0; evt < 50000; evt++) {
    if (r.Uniform(0, 1) < 0.65)
      Ex = r.Gaus(950, 70);
    else
      Ex = r.Gaus(1350, 90);

    tree->Fill();
    h1->Fill(Ex);
  }

  tree->Write();

  histDir->cd();
  h1->Write();

  fout->Close();
}
