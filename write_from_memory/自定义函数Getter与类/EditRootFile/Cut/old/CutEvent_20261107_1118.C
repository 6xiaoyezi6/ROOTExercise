// 功能：读取 alphap.root，将通过 alpha cut 的事件写入 alphap_cut.root。
// 方法：通过 ReadEvent、Gate 指针逐事件读取和判选，以首条 (ΔE, E) 记录筛选并复制全部分支。
// 注意事项：依赖 NPTool 和 alphaCut.C；运行覆盖输出文件，结束时先 delete cut，再 delete reader。
#include "Gate.h"
#include "TClusterGAGGData.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include <iostream>

#ifdef __CLING__
R__LOAD_LIBRARY(libNPClusterGAGG)
#endif

void CutEvent()
{
  TString InputRootPath = "alphap.root";
  TString InputTreeName = "SimulatedTree";
  ReadEvent *reader = new ReadEvent(InputRootPath, InputTreeName);
  TTree *tree = reader->GetInputTree();

  TString CutPath = "alphaCut.C";
  Gate *cut = new Gate(CutPath, *reader);

  TString OutputRootPath = "alphap_cut.root";
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
