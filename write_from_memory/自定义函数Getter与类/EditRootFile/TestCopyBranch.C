// 功能：从 Ex5sigma.root 复制指定分支和全部分支，并绘制 ExWithoutCut 谱。
// 方法：CopyBranch 和 DrawBranch 继承 CopyAllBranch，共用文件和树的管理；
//       使用 DrawBranch 的事件读取接口和 getter 逐事件填充 TH1D。
// 注意事项：需配置 NPTool 环境和 libNPTimer；运行会覆盖两个输出 ROOT 文件。
//           直方图脱离文件管理，关闭文件后仍可显示；分箱和范围在函数开头调整。
#include "RtypesCore.h"
#ifdef __CLING__
R__LOAD_LIBRARY(libNPTimer)
#endif
#include "CopyBranch.h"
#include "TString.h"
#include "TCanvas.h"
#include "TH1D.h"

void TestCopyBranch() {

  TString InputRootPath = "Ex5sigma.root";
  TString SelectedRootPath = "ExSelected.root";
  TString AllRootPath = "ExAll.root";
  TString InputTreeName = "tree";
  TString OutputTreeName = "tree";

  CopyAllBranch *copyAll = new CopyAllBranch(InputRootPath, AllRootPath,InputTreeName, OutputTreeName);
  CopyBranch *copy = new CopyBranch(InputRootPath, SelectedRootPath,InputTreeName, OutputTreeName);
  DrawBranch *drawbranch = new DrawBranch(InputRootPath, InputTreeName);

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

  // 删除派生类对象时会自动调用基类析构函数，统一释放文件
  delete copyAll;
  delete copy;
  delete drawbranch;
}
