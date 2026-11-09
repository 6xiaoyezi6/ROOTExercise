// 功能：读取 alphap.root，绘制 ClusterGAGG 与 DeltaEGAGG 的首条能量记录关联图。
// 方法：按本宏所在目录定位输入，绑定 ClusterGAGG 分支，逐事件填充二维直方图。
// 注意事项：需配置 ROOT、NPTool 和 libNPClusterGAGG；两个能量记录均非空才填图，能量单位为 MeV。
#include "TCanvas.h"
#include "TClusterGAGGData.h"
#include "TFile.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"
#include <iostream>

#ifdef __CLING__
R__LOAD_LIBRARY(libNPClusterGAGG)
#endif

void Reconstruct()
{
  TString dir = gSystem->DirName(__FILE__);
  TString InputRootPath = dir + "/alphap.root";
  TFile *f = TFile::Open(InputRootPath.Data(), "READ");
  TTree *tree = (TTree*)f->Get("SimulatedTree");

  TClusterGAGGData *cg = nullptr;
  tree->SetBranchAddress("ClusterGAGG", &cg);

  // 分箱及显示范围可在这里调整，当前范围包含输入中的全部首记录配对。
  TH2F *h1 = new TH2F("h1", ";ClusterGAGG E (MeV);DeltaEGAGG #DeltaE (MeV);Counts",
                      800, 0, 800, 650, 0, 650);
  h1->SetDirectory(nullptr);

  Long64_t nentries = tree->GetEntries();
  for (Long64_t i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    if (cg->GetClusterGAGGMultEnergy() == 0 || cg->GetDeltaEGAGGMultEnergy() == 0)
      continue;

    Double_t ClusterGAGGEnergy = cg->GetClusterGAGGEnergy(0);
    Double_t DeltaEGAGGEnergy = cg->GetDeltaEGAGGEnergy(0);
    h1->Fill(ClusterGAGGEnergy, DeltaEGAGGEnergy);
  }
  f->Close();
  delete f;


  TCanvas *c1 = new TCanvas("c1", "ClusterGAGG - DeltaEGAGG", 1000, 700);
  h1->Draw("COLZ");
  c1->Modified();
  c1->Update();

}
