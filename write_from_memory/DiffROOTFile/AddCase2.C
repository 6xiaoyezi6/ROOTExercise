/*
功能：按 EventID 合并 Case2_1.root 的 x、y 和 Case2_2.root 的 z，输出 Case2.root。
方法：以 Case2_1.root 的 tree 为主表，为第二棵 tree 建立 EventID 索引并逐事件查找。
注意事项：保留主表全部事件；第二棵树没有对应 EventID 时将 z 设为 -999；输出文件会被覆盖。
*/
#include "TFile.h"
#include "TTree.h"

void AddCase2() {
  TFile *f1 = new TFile("Case2_1.root", "READ");
  TFile *f2 = new TFile("Case2_2.root", "READ");
  TTree *t1 = (TTree *)f1->Get("tree");
  TTree *t2 = (TTree *)f2->Get("tree");

  Int_t EventID1, EventID2;
  Double_t x, y, z;
  const Double_t kInvalidValue = -999.0;

  t1->SetBranchAddress("EventID", &EventID1);
  t1->SetBranchAddress("x", &x);
  t1->SetBranchAddress("y", &y);

  t2->SetBranchAddress("EventID", &EventID2);
  t2->SetBranchAddress("z", &z);

  t2->BuildIndex("EventID");

  TFile *fout = new TFile("Case2.root", "RECREATE");
  TTree *tout = new TTree("tree", "Merged tree");

  tout->Branch("EventID", &EventID1, "EventID/I");
  tout->Branch("x", &x, "x/D");
  tout->Branch("y", &y, "y/D");
  tout->Branch("z", &z, "z/D");

  for (Long64_t i = 0; i < t1->GetEntries(); i++) {
    t1->GetEntry(i);
    z = kInvalidValue;//初始化
    //拿当前t1的EventID1去t2中找EventID2==EventID1的事件。如果找到了，t2当前事件的数据会被读取
    Long64_t entry = t2->GetEntryWithIndex(EventID1);
    //如果查找失败，或者读出来的EventID2和EventID1不一致，就把z重新设为 -999
    if (entry <= 0 || EventID2 != EventID1) z = kInvalidValue;
    tout->Fill();
  }

  tout->Write();
  fout->Close();
  f1->Close();
  f2->Close();
}
