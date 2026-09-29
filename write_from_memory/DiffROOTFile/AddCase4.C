/*
功能：纵向拼接 Case3_1.root 和 Case3_2.root，输出 Case4.root。
方法：先写入第一棵 tree 的全部事件，再写入第二棵 tree 的全部事件。
注意事项：某个输入文件缺失的分支统一写为 -999；输出文件会被覆盖。
*/
#include "TFile.h"
#include "TTree.h"

void AddCase4() {
  TFile *f1 = new TFile("Case3_1.root", "READ");
  TFile *f2 = new TFile("Case3_2.root", "READ");
  TTree *t1 = (TTree *)f1->Get("tree");
  TTree *t2 = (TTree *)f2->Get("tree");

  Int_t EventID;
  Double_t x, y, t, z, u;
  const Double_t kInvalidValue = -999.0;

  t1->SetBranchAddress("EventID", &EventID);
  t1->SetBranchAddress("x", &x);
  t1->SetBranchAddress("y", &y);
  t1->SetBranchAddress("t", &t);

  t2->SetBranchAddress("EventID", &EventID);
  t2->SetBranchAddress("z", &z);
  t2->SetBranchAddress("u", &u);

  TFile *fout = new TFile("Case4.root", "RECREATE");
  TTree *tout = new TTree("tree", "Merged tree");

  tout->Branch("EventID", &EventID, "EventID/I");
  tout->Branch("x", &x, "x/D");
  tout->Branch("y", &y, "y/D");
  tout->Branch("t", &t, "t/D");
  tout->Branch("z", &z, "z/D");
  tout->Branch("u", &u, "u/D");

  for (Long64_t i = 0; i < t1->GetEntries(); i++) {
    t1->GetEntry(i);
    z = kInvalidValue;
    u = kInvalidValue;
    tout->Fill();
  }

  for (Long64_t i = 0; i < t2->GetEntries(); i++) {
    t2->GetEntry(i);
    x = kInvalidValue;
    y = kInvalidValue;
    t = kInvalidValue;
    tout->Fill();
  }

  tout->Write();
  fout->Close();
  f1->Close();
  f2->Close();
}