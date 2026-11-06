// 功能：从 Ex5sigma.root 复制指定分支和全部分支，并绘制 ExWithoutCut 谱。
// 方法：使用 CopyBranch 的事件读取接口和 getter 逐事件填充 TH1D。
// 注意事项：需配置 NPTool 环境和 libNPTimer；运行会覆盖两个输出 ROOT 文件。
//           直方图脱离文件管理，关闭文件后仍可显示；分箱和范围在函数开头调整。
#include "RtypesCore.h"
#ifdef __CLING__
R__LOAD_LIBRARY(libNPTimer)
#endif
#include "MyTools/TTimer.h"
#include "CopyBranch.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TMath.h"
#include "TStyle.h"

void TestCopyBranch() {

  CopyBranch *copy = new CopyBranch("Ex5sigma.root", "ExSelected.root", "tree", "tree");
  CopyAllBranch *copyAll = new CopyAllBranch("Ex5sigma.root", "ExAll.root", "tree", "tree");
  DrawBranch *drawbranch = new DrawBranch("Ex5sigma.root","tree");


  TH1D *h1 = new TH1D("h1", "", 500, -20, 30);
  h1->SetDirectory(nullptr);

  Long64_t nentries = drawbranch->GetInputEntries();
  for (Long64_t i = 0; i < nentries; i++) {
    drawbranch->GetInputEntry(i);
    h1->Fill(drawbranch->GetExWithoutCut());
  }
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  h1->Draw("HIST");
  c1->SaveAs("aa.pdf");

 
  delete copyAll;
  delete copy;
  delete drawbranch;

}
