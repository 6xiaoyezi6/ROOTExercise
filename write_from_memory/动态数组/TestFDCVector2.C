#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <vector>

void TestFDCVector2() {
  TFile *f = TFile::Open("ppBe10.root_convert", "READ");
  TTree *tree = (TTree *)f->Get("tree");

  std::vector<double> *FDC0_Track_dl = 0;
  tree->SetBranchAddress("FDC0_Track_dl", &FDC0_Track_dl);

  Long64_t nentries = tree->GetEntriesFast();
  for (Long64_t event = 0; event < nentries; ++event) {
    tree->GetEntry(event);
    if (FDC0_Track_dl->size() > 8) {

      std::cout << "Event " << event << std::endl;
      std::cout << "  Number of elements: " << FDC0_Track_dl->size()<< std::endl;

      for (size_t index = 0; index < FDC0_Track_dl->size(); ++index) {
        std::cout << "  [" << index << "] = " << FDC0_Track_dl->at(index)
                  << std::endl;
      }
    }
  }

  f->Close();
}
