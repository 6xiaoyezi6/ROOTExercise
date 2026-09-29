#include "TFile.h"
#include "TTree.h"

void AddCase1() {
  TFile *f1 = new TFile("Case1_1.root", "READ");
  TFile *f2 = new TFile("Case1_2.root", "READ");
  TTree *t1 = (TTree *)f1->Get("tree");
  TTree *t2 = (TTree *)f2->Get("tree");

  Int_t EventID1, EventID2;
  Double_t x, y, z;

  t1->SetBranchAddress("EventID", &EventID1);
  t1->SetBranchAddress("x", &x);
  t1->SetBranchAddress("y", &y);

  t2->SetBranchAddress("EventID", &EventID2);
  t2->SetBranchAddress("z", &z);

  t2->BuildIndex("EventID"); // 关键，根据人为定义的EventID进行事件匹配

  TFile *fout = new TFile("Case1.root", "RECREATE");
  TTree *tout = new TTree("tree", "Merged tree");

  tout->Branch("EventID", &EventID1, "EventID/I");
  tout->Branch("x", &x, "x/D");
  tout->Branch("y", &y, "y/D");
  tout->Branch("z", &z, "z/D");

  for (Long64_t i = 0; i < t1->GetEntries(); i++) {
    t1->GetEntry(i);//读取这个entry中所有当前激活并且已经绑定的branch数据

    //以t1为主表，逐事件读取t1，根据EventID去t2找匹配事件，找到后合并写入tout
    Long64_t entry = t2->GetEntryWithIndex(EventID1); // 关键
    if (entry < 0) continue;

    tout->Fill();
  }

  tout->Write();
  fout->Close();
  f1->Close();
  f2->Close();
}
