#include "TFile.h"
#include "TTree.h"

void GenerateCase1_2Data() {
  TFile *fout = new TFile("Case1_2.root", "RECREATE");
  TTree *tree = new TTree("tree", "Case1_2 tree");

  Int_t EventID;
  Double_t z;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("z", &z, "z/D");

  for (Int_t i = 0; i < 10; i++) {
    EventID = i;
    z = 4 * i;

    tree->Fill();
  }

  tree->Write();
  fout->Close();
}
