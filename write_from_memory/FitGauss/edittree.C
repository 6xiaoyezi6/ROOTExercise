#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVector3.h"

void edittree() {
  TFile *f = new TFile("FitGaussData.root", "UPDATE");
  TTree *t = (TTree *)f->Get("tree");

  Float_t Ex;
  Float_t newEx;

  t->SetBranchAddress("Ex", &Ex);
  TTree *tout = t->CloneTree(0);
  tout->SetName("newtree"); //给复制之后的tree重命名
  tout->Branch("newEx", &newEx, "newEx/F");

  TCanvas *c = new TCanvas("c", "c", 800, 600);
  TH1F *h1 = new TH1F("h1", "newEx;new Ex;counts", 2000, 0, 5000);
  h1->SetDirectory(0);//

  Long64_t nentries = t->GetEntriesFast();
  for (Long64_t i = 0; i < nentries; i++) 
  {
    t->GetEntry(i);
    newEx = Ex * 2 + 1000;

    h1->Fill(newEx); // TH1F	h1->Fill(Ex);	把某一个数值填入直方图
    tout->Fill(); // TTree	tree->Fill();	把所有已经绑定的 branch 变量当前值存成一条事件
                  
              
  }

  h1->Draw();
  f->cd();
  tout->Write();//不能少
  f->Close();
}
