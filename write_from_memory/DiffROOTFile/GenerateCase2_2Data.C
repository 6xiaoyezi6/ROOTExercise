#include "TFile.h"
#include "TTree.h"

void GenerateCase2_2Data() {
  TFile *fout = new TFile("Case2_2.root", "RECREATE");
  TTree *tree = new TTree("tree", "Case2_2 tree");

  Int_t EventID;
  Double_t z;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("z", &z, "z/D");


  //只填写奇数的
  for (Int_t i = 0; i < 10; i+=2) {
    EventID = i;
    z = 4 * i;

    tree->Fill();
  }

  tree->Write();
  fout->Close();
}
