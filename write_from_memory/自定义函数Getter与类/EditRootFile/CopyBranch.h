// 功能：读取 ROOT 文件和 TTree，复制全部或指定分支，并提供 ExWithoutCut 的读取接口。
// 方法：ReadTree 管理输入；CopyAllBranch 继承 ReadTree，管理输出并复制树；
//       CopyBranch 和 DrawBranch 继承 CopyAllBranch，分别选择分支和绑定分支。
// 注意事项：需配置 ROOT、NPTool 和 libNPTimer；输入树和分支需存在，复制会覆盖输出文件。
//           先调用 GetInputEntry() 读取事件，再调用 getter；输入文件由 ReadTree 释放。
#ifndef COPYBRANCH_H
#define COPYBRANCH_H

#include "RtypesCore.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "MyTools/TTimer.h"
#include <iostream>

/* 类的继承关系
class ReadTree
└── class CopyAllBranch
    ├── class CopyBranch
    └── class DrawBranch
*/

class ReadTree {
public:
  TFile *InputFile = nullptr;
  TTree *InputTree = nullptr;

  ReadTree(TString InputRootPath, TString InputTreeName) 
  {
    InputFile = TFile::Open(InputRootPath.Data(), "READ");
    InputTree = (TTree *)InputFile->Get(InputTreeName.Data());
  }

  // 下面这两个不能合并在一起
  Long64_t GetInputEntries() { return InputTree->GetEntriesFast(); }
  void GetInputEntry(Long64_t entry) { InputTree->GetEntry(entry); }

  //通过父类指针删除子类对象，要使用virtual
  virtual ~ReadTree() {
    delete InputFile;
  }
};

/////////////////////////////////////////////////////////////////////

class CopyAllBranch : public ReadTree {
public:
  TFile *OutputFile = nullptr;
  TTree *OutputTree = nullptr;

  // 供派生类调用：只打开输入，不复制树
  //子类调用父类的构造函数，使用C++的「构造函数成员初始化列表」
  CopyAllBranch(TString InputRootPath, TString InputTreeName)
      : ReadTree(InputRootPath, InputTreeName) {
  }

  // 复制当前启用的 Branch 以及全部 Event
  void CopyTree(TString OutputRootPath, TString OutputTreeName) {
    OutputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");
    OutputFile->cd();
    OutputTree = InputTree->CloneTree(-1, "fast");//-1 表示复制全部 Event
    OutputTree->SetName(OutputTreeName.Data());
    OutputTree->Write();
  }

  //构造函数重载：同一个类可以有多个构造函数，只要参数列表不同
  //先调用前面定义的两参数构造函数，完成输入ROOT文件的打开，然后执行四参数的函数(初始化列表)
  CopyAllBranch(TString InputRootPath, TString OutputRootPath,
                TString InputTreeName, TString OutputTreeName)
      : CopyAllBranch(InputRootPath, InputTreeName) {
    // 启用所有 Branch
    InputTree->SetBranchStatus("*", 1);
    CopyTree(OutputRootPath, OutputTreeName);
  }

  // 输入文件由 ReadTree 的析构函数释放
  ~CopyAllBranch() override {
    delete OutputFile;
  }
};

/////////////////////////////////////////////////////////////////////

class CopyBranch : public CopyAllBranch {
private:
  NPL::TTimer *timer = new NPL::TTimer();

public:
  CopyBranch(TString InputRootPath, TString OutputRootPath,
             TString InputTreeName, TString OutputTreeName)
      : CopyAllBranch(InputRootPath, InputTreeName) //注意，不能直接调用ReadTree
  {
    // 关闭所有 Branch
    InputTree->SetBranchStatus("*", 0);

    // 启用需要复制的 Branch
    InputTree->SetBranchStatus("ExWithoutCut", 1);
    InputTree->SetBranchStatus("ExWithCutZ_BDCRDC", 1);
    InputTree->SetBranchStatus("protonTheta", 1);

    CopyTree(OutputRootPath, OutputTreeName);
  }

  ~CopyBranch() override {
    timer->Stop();
    delete timer;
  }
};

/////////////////////////////////////////////////////////////////////

class DrawBranch : public CopyAllBranch {
private:
  Double_t ExWithoutCut;

public:
  DrawBranch(TString InputRootPath, TString InputTreeName)
      : CopyAllBranch(InputRootPath, InputTreeName) {
    // 绑定分支
    InputTree->SetBranchAddress("ExWithoutCut", &ExWithoutCut);
  }

  Double_t GetExWithoutCut() { return ExWithoutCut; }
};

#endif
