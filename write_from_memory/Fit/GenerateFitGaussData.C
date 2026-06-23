#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"

void GenerateFitGaussData() {
  TFile *fout = TFile::Open("FitGaussData.root", "RECREATE");
  TDirectory *histDir = fout->mkdir("histograms");//下面的就进入histograms中
  TTree *tree = new TTree("tree", "simple event tree", 99,gDirectory); // tree放入默认的gDirectory中
  //TTree(const char *name, const char *title, Int_t splitlevel = 99, TDirectory *dir = gDirectory)
  //split level，控制对象型 branch 是否自动拆成子 branch。默认 99 表示尽量自动拆分(不用改)
  //TDirectory *dir = gDirectory 指定 tree 属于哪个 ROOT 目录。通常就是当前目录 gDirectory

  Int_t evt;
  Double_t Ex;

  tree->Branch("evt", &evt, "evt/I");
  tree->Branch("Ex", &Ex, "Ex/D");

  TCanvas *c = new TCanvas("c", "c", 800, 600);
  TH1F *h1 = new TH1F("h1", "Ex;Ex(adc);counts", 1000, 0, 2000);

  TRandom r(0);
  for (evt = 0; evt < 50000; evt++) {
    if (r.Uniform(0, 1) < 0.65)
      Ex = r.Gaus(950, 70);
    else
      Ex = r.Gaus(1350, 90);

    tree->Fill(); // 在事件循环中填入(把Ex填入tree的branch中)
                  // h1->Fill(Ex);把某一个数值填入直方图
    h1->Fill(Ex); // 把TH1F写入root文件中的TDirectory文件夹中
                  // tree->Fill();把所有已经绑定的branch变量当前值存成一条事件
  }

  tree->Write(); // 写到 FitGaussData.root:/tree (tree已经绑到默认的gDirectory中了)

  histDir->cd(); // 切换到 histograms 子目录
  h1->Write();   // 写到 FitGaussData.root:/histograms/h1

  fout->Close(); // 完成写入后关闭文件
}
