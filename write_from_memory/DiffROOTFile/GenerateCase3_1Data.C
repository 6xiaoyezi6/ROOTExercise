/*
功能：生成情况 3 的主输入 Case3_1.root，包含 EventID、x、y、t 四个分支。
方法：写入 EventID 为 0 到 9 的十条事件，令 x=i、y=2*i、t=3*i。
注意事项：输出树名为 tree；再次运行会覆盖 Case3_1.root。
*/
#include "TFile.h"
#include "TTree.h"

void GenerateCase3_1Data() {
  TFile *fout = new TFile("Case3_1.root", "RECREATE");
  TTree *tree = new TTree("tree", "Case3_1 tree");

  Int_t EventID;
  Double_t x, y, t;

  tree->Branch("EventID", &EventID, "EventID/I");
  tree->Branch("x", &x, "x/D");
  tree->Branch("y", &y, "y/D");
  tree->Branch("t", &t, "t/D");

  for (Int_t i = 0; i < 15; i++) {
    EventID = i;
    x = i;
    y = 2 * i;
    t = 3 * i;
    tree->Fill();
  }

  tree->Write();
  fout->Close();
}
