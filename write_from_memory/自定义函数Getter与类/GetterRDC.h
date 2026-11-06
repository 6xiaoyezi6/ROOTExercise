/*
 * 功能：封装 ESPRI 标定树读取，提供左右 RDC 的坐标和斜率 Getter。
 * 方法：接收 TString 路径，以 TFile::Open(path.Data(), "READ") 打开文件并绑定四个数组分支。
 * 注意事项：输入须含 CalTreeESPRI 及长度为 2 的 Double_t 数组；side 仅允许 0、1。
 *           先调用 GetEntry 再使用 Getter；对象独占输入文件，析构时关闭。
 */
#ifndef GETTERRDC_H
#define GETTERRDC_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>

//这里里面的Getter函数是有参数的
class GetterRDCReader {
private:
  TFile *inputFile;
  TTree *calTree;

  Double_t rdcA[2];
  Double_t rdcB[2];
  Double_t rdcX[2];
  Double_t rdcY[2];

public:
  GetterRDCReader(const TString &InputRootPath) {
    inputFile = TFile::Open(InputRootPath.Data(), "READ");
    calTree = (TTree*)inputFile->Get("CalTreeESPRI");

    calTree->SetBranchAddress("rdcA", rdcA);
    calTree->SetBranchAddress("rdcB", rdcB);
    calTree->SetBranchAddress("rdcX", rdcX);
    calTree->SetBranchAddress("rdcY", rdcY);
  }

  ~GetterRDCReader() {
    inputFile->Close();
    delete inputFile;
  }

  Long64_t GetEntries() { return calTree->GetEntries(); }

  void GetEntry(Long64_t entry) { calTree->GetEntry(entry); }

  Double_t GetrdcA(Int_t side) { return rdcA[side]; }
  Double_t GetrdcB(Int_t side) { return rdcB[side]; }
  Double_t GetrdcX(Int_t side) { return rdcX[side]; }
  Double_t GetrdcY(Int_t side) { return rdcY[side]; }

  Double_t GetLeftrdcA() { return rdcA[0]; }
  Double_t GetRightrdcA() { return rdcA[1]; }
};

#endif
