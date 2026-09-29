/*
功能：按 EventID 合并 Case3_1.root 的 x、y、t 与 Case3_2.root 的 z、u，输出 Case3.root。
方法：以第一棵 tree 为主表，为第二棵 tree 建立 EventID 索引并逐事件查找。
注意事项：保留主表全部事件；缺少对应事件时 z 和 u 均写为 -999；输出文件会被覆盖。
*/
#include "TFile.h"
#include "TTree.h"

void AddCase3() {
  TFile *f1 = new TFile("Case3_1.root", "READ");
  TFile *f2 = new TFile("Case3_2.root", "READ");
  TTree *t1 = (TTree *)f1->Get("tree");
  TTree *t2 = (TTree *)f2->Get("tree");

  Int_t EventID1, EventID2;
  Double_t x, y, t, z, u;
  const Double_t kInvalidValue = -999.0;

  t1->SetBranchAddress("EventID", &EventID1);
  t1->SetBranchAddress("x", &x);
  t1->SetBranchAddress("y", &y);
  t1->SetBranchAddress("t", &t);

  t2->SetBranchAddress("EventID", &EventID2);
  t2->SetBranchAddress("z", &z);
  t2->SetBranchAddress("u", &u);

  t2->BuildIndex("EventID");

  TFile *fout = new TFile("Case3.root", "RECREATE");
  TTree *tout = new TTree("tree", "Merged tree");

  tout->Branch("EventID", &EventID1, "EventID/I");
  tout->Branch("x", &x, "x/D");
  tout->Branch("y", &y, "y/D");
  tout->Branch("t", &t, "t/D");
  tout->Branch("z", &z, "z/D");
  tout->Branch("u", &u, "u/D");

  for (Long64_t i = 0; i < t1->GetEntries(); i++) {
    t1->GetEntry(i);
    z = kInvalidValue;
    u = kInvalidValue;
    Long64_t entry = t2->GetEntryWithIndex(EventID1);
    if (entry <= 0 || EventID2 != EventID1) {
      z = kInvalidValue;
      u = kInvalidValue;
    }
    tout->Fill();
  }

  tout->Write();
  fout->Close();
  f1->Close();
  f2->Close();
}
