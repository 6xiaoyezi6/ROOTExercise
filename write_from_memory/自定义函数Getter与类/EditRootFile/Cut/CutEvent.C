// 功能：读取 alphap.root，将通过 alpha cut 的事件写入 alphap_cut.root。
// 方法：直接使用 reader->InputTree，Gate 通过公开的 reader->cg 按首条 (E, ΔE) 记录判选；路径按本宏所在目录拼接。
// 注意事项：依赖 NPTool 和 alphaCut.C；运行覆盖输出文件，结束时先 delete cut，再 delete reader。
#include "Gate.h"
#include "TClusterGAGGData.h"
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include <iostream>

#ifdef __CLING__
R__LOAD_LIBRARY(libNPClusterGAGG)
#endif

void CutEvent()
{
  TString dir = gSystem->DirName(__FILE__);
  TString InputRootPath = dir + "/alphap.root";
  TString InputTreeName = "SimulatedTree";
  ReadEvent *reader = new ReadEvent(InputRootPath, InputTreeName);
  TTree *tree = reader->InputTree;

  TString CutPath = dir + "/alphaCut.C";
  Gate *cut = new Gate(CutPath, *reader);

  TString OutputRootPath = dir + "/alphap_cut.root";
  TFile *OutputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");
  OutputFile->cd();
  TTree *OutputTree = tree->CloneTree(0);
  //参数0表示只复制原始TTree的结构（Branch），不复制任何事件数据
  //参数-1表示复制TTree结构和全部事件

  Long64_t nentries = reader->GetInputEntries();
  for (Long64_t i = 0; i < nentries; i++) {
    reader->GetInputEntry(i);
    if (!reader->HasEnergy()) continue;
    if (cut->IsAlpha()) OutputTree->Fill();
  }

  OutputFile->cd();
  OutputTree->Write();
  OutputFile->Close();
  delete OutputFile;
  delete cut;
  delete reader;
}
