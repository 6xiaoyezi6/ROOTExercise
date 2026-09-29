/*
功能：生成情况 3 的第二个输入 Case3_2.root，包含 EventID、z、u 三个分支。
方法：仅写入 EventID 为 1、3、5、7、9 的事件，令 z=4*i、u=5*i。
注意事项：输出树名为 tree；再次运行会覆盖 Case3_2.root。
*/
#include "TFile.h"
#include "TTree.h"

void GenerateCase3_2Data() {
  TFile *fout = new TFile("Case3_2.root", "RECREATE");
  TTree *tree = new TTree("tree", "Case3_2 tree");

  Int_t EventID;
  Double_t z, u;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("z", &z, "z/D");
  tree->Branch("u", &u, "u/D");

  for (Int_t i = 1; i < 20; i += 2) {
    EventID = i;
    z = 4 * i;
    u = 5 * i;
    tree->Fill();
  }

  tree->Write();
  fout->Close();
}
