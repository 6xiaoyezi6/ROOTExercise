#ifndef COPYBRANCH_H
#define COPYBRANCH_H

#include "RtypesCore.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include <iostream>

class CopyBranch {
private:
  NPL::TTimer timer;

  TFile *InputFile = nullptr;
  TTree *InputTree = nullptr;
  TFile *OutputFile = nullptr;
  TTree *OutputTree = nullptr;

public:
  CopyBranch(TString InputRootPath,
             TString OutputRootPath, // 函数参数的名字不能和成员变量一致
             TString InputTreeName, TString OutputTreeName) {

    InputFile = TFile::Open(InputRootPath.Data(), "READ");
    InputTree = (TTree *)InputFile->Get(InputTreeName.Data());

    // 关闭所有 Branch
    InputTree->SetBranchStatus("*", 0);

    // 启用需要复制的 Branch
    InputTree->SetBranchStatus("ExWithoutCut", 1);
    InputTree->SetBranchStatus("ExWithCutZ_BDCRDC", 1);
    InputTree->SetBranchStatus("protonTheta", 1);

    // 创建输出文件
    OutputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");
    OutputFile->cd();

    // 复制启用的 Branch 以及全部 Event
    OutputTree = InputTree->CloneTree(-1, "fast");

    // 设置输出 TTree 名称并写入
    OutputTree->SetName(OutputTreeName.Data());
    OutputTree->Write();
  }

  ~CopyBranch() {
    delete OutputFile;
    delete InputFile;
    timer.Stop();
  }

};



/////////////////////////////////////////////////////////////////////

class CopyAllBranch {
private:
  TFile *InputFile = nullptr;
  TTree *InputTree = nullptr;
  TFile *OutputFile = nullptr;
  TTree *OutputTree = nullptr;

public:
  CopyAllBranch(TString InputRootPath, TString OutputRootPath,
                TString InputTreeName, TString OutputTreeName) {

    InputFile = TFile::Open(InputRootPath.Data(), "READ");
    InputTree = (TTree *)InputFile->Get(InputTreeName.Data());

    // 启用所有 Branch
    InputTree->SetBranchStatus("*", 1);

    // 创建输出文件
    OutputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");
    OutputFile->cd();

    // 复制整个 TTree，包括全部 Branch 和 Event
    OutputTree = InputTree->CloneTree(-1, "fast");

    // 设置输出 TTree 名称并写入
    OutputTree->SetName(OutputTreeName.Data());
    OutputTree->Write();
  }

  ~CopyAllBranch() {
    delete OutputFile;
    delete InputFile;
  }
};



/////////////////////////////////////////////////////////////////////////////

class DrawBranch {
private:

  TFile *InputFile = nullptr;
  TTree *InputTree = nullptr;

  Double_t ExWithoutCut;

public:
  DrawBranch(TString InputRootPath,TString InputTreeName) 
  {

    InputFile = TFile::Open(InputRootPath.Data(), "READ");
    InputTree = (TTree *)InputFile->Get(InputTreeName.Data());

    // 绑定分支
    InputTree->SetBranchAddress("ExWithoutCut", &ExWithoutCut);
  }

  ~DrawBranch() {
    delete InputFile;
  }
  //下面这两个不能合并在一起
  Long64_t GetInputEntries() { return InputTree->GetEntriesFast(); }
  void GetInputEntry(Long64_t entry) { InputTree -> GetEntry(entry); }

  Double_t GetExWithoutCut() { return ExWithoutCut; }
};



#endif
