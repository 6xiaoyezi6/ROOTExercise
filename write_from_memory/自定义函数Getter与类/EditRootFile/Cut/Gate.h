// 功能：读取 ClusterGAGG 分支，并用 AlphaCUTG 判断首条能量记录是否为 alpha。
// 方法：ReadEvent 继承 ReadTree，Gate 通过 reader->cg 获取 E、ΔE 并判选；两个类的成员均为 public。
// 注意事项：需配置 ROOT 和 NPTool；先读取事件并通过 HasEnergy()，再调用 IsAlpha()。
//           cut 宏需创建 AlphaCUTG；先释放 Gate，再释放 ReadEvent，cg 由 ReadEvent 释放，输入文件由 ReadTree 释放。
#ifndef GATE_H
#define GATE_H

#include "../ReadTree.h"
#include "TClusterGAGGData.h"
#include "TCutG.h"
#include "TFile.h"
#include "TList.h"
#include "TROOT.h"
#include "TString.h"
#include "TTree.h"
#include <iostream>

class ReadEvent : public ReadTree {
public:
  TClusterGAGGData *cg = nullptr;

  ReadEvent(TString InputRootPath, TString InputTreeName)
      : ReadTree(InputRootPath, InputTreeName)
  {
    InputTree->SetBranchAddress("ClusterGAGG", &cg);
  }

  ~ReadEvent() override
  {
    delete cg;
  }

  TTree *GetInputTree() { return InputTree; }

  Bool_t HasEnergy()
  {
    if (cg == nullptr)
      return false;
    if (cg->GetClusterGAGGMultEnergy() == 0 || cg->GetDeltaEGAGGMultEnergy() == 0)
      return false;
    return true;
  }
};

//////////////////////////////////////////////////////////////////////
class Gate {
public:
  ReadEvent *reader = nullptr;
  TCutG *AlphaCut = nullptr;

  void LoadCut(TString CutPath) 
  {
    TString CutLine = ".x " + CutPath;
    gROOT->ProcessLine(CutLine.Data());
  }

  void GetCut() 
  {
    AlphaCut = (TCutG*)gROOT->GetListOfSpecials()->FindObject("AlphaCUTG");
  }

  Gate(TString CutPath, ReadEvent &InputReader)
  {
    reader = &InputReader;
    LoadCut(CutPath);
    GetCut();
  }

  Bool_t IsAlpha() {
    Double_t E = reader->cg->GetClusterGAGGEnergy(0);
    Double_t DE = reader->cg->GetDeltaEGAGGEnergy(0);
    return AlphaCut->IsInside(E, DE);
  }
};

#endif
