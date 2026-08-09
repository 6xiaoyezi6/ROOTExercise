#include <string>

#include "TFile.h"
#include "TRandom3.h"
#include "TString.h"
#include "TTree.h"

//创建两个分支，side1和side2，一个用标准 C++ 的 std::string，一个用ROOT 的TString
// std::string 在TBrowser中用c_str()查看
//TString 用 .Data()查看

void StringData() {
  TFile *outputFile = new TFile("StringData.root", "RECREATE");
  TTree *tree = new TTree("tree", "String data");

  std::string side1;
  TString side2;

  tree->Branch("side1", &side1);
  tree->Branch("side2", &side2);

  TRandom3 random(0);

  for (int i = 0; i < 1000; i++) 
  {
    if (random.Integer(2) == 0) 
    {
      side1 = "Left";
      side2 = "Left";
    } 
    else 
    {
      side1 = "Right";
      side2 = "Right";
    }

    tree->Fill();
  }

  outputFile->cd();
  tree->Write();
  outputFile->Close();
}
