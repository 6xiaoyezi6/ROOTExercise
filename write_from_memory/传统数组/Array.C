#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TTree.h"

// 来测试ROOT是如何显示数组
// 下面产生一个长度为4的数组E[4]
// 每一个事件，数组的第一二三四个位置依次写入一个高斯随机数
// tree->Draw("E")的输入是2万，是5000*4，把数组的每一个元素都显示出来了
// tree->Draw("E[1]") 这是显示的是一个事件中数组中某一个元素的分布情况

//TTree 中保存 E[4] 四个高斯峰；
//TH1F h 中保存 四个高斯峰 + 均匀本底

void Array() {
  TFile *f = TFile::Open("Array.root", "RECREATE");
  TTree *tree = new TTree("tree", "Array Tree");

  Double_t E[4];

  tree->Branch("E", E, "E[4]/D");

  TRandom r(0);

  for (Int_t i = 0; i < 5000; i++) {
    E[0] = r.Gaus(0, 1);
    E[1] = r.Gaus(5, 1);
    E[2] = r.Gaus(10, 1);
    E[3] = r.Gaus(15, 1);
    tree->Fill();
  }

  TH1F *h = new TH1F("h","Spectrum;Energy;Counts",250,-5,20);
  // 四个高斯峰
  for (Int_t i = 0; i < 5000; i++) 
  {
    h->Fill(r.Gaus(0, 1));
    h->Fill(r.Gaus(5, 1));
    h->Fill(r.Gaus(10, 1));
    h->Fill(r.Gaus(15, 1));
  }

  TF1*fpol = new TF1("fpol","2*x*x+x+2",0,20);
  // 均匀本底
  for (Int_t i = 0; i < 10000; i++) 
  {
    //h->Fill(r.Uniform(-5, 20));
    h->Fill(fpol->GetRandom());
  }

  tree->Write();
  h->Write();
  f->Close();
}
