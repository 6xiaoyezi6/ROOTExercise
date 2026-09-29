#include "TFile.h"
#include "TTree.h"

void GenerateCase2_1Data() {
  TFile *fout = new TFile("Case2_1.root", "RECREATE");
  TTree *tree = new TTree("tree", "Case2_1 tree");

  Int_t EventID;
  Double_t x;
  Double_t y;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("x", &x, "x/D");
  tree->Branch("y", &y, "y/D");

  for (Int_t i = 0; i < 10; i++) {
    EventID = i;
    x = i;
    y = 2 * x;

    tree->Fill();
  }

  tree->Write();
  fout->Close();
}
