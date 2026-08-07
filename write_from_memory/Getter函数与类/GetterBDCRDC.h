#ifndef GETTERBDCRDC_H
#define GETTERBDCRDC_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

#include <vector>

// BDC 基类：负责打开 BDC 文件、连接分支和提供 BDC Getter。
class GetterBDCBase {
private:
  TFile *bdcInputFile;
  TTree *bdcCalTree;
  TTree *bdcRawTree;

  Double_t BDC1_ThetaX;
  Double_t BDC1_ThetaY;
  Double_t BDC1_X;
  Double_t BDC1_Y;
  Double_t BDC2_ThetaX;
  Double_t BDC2_ThetaY;
  Double_t BDC2_X;
  Double_t BDC2_Y;

  std::vector<double> *bdc1_drift;

public:
  GetterBDCBase(const char *bdcFileName) {
    bdcInputFile = TFile::Open(bdcFileName, "READ");
    bdcCalTree =
        static_cast<TTree *>(bdcInputFile->Get("CalTreeBDC"));
    bdcRawTree =
        static_cast<TTree *>(bdcInputFile->Get("RawBDCTree"));

    bdc1_drift = nullptr;

    bdcCalTree->SetBranchAddress("BDC1_ThetaX", &BDC1_ThetaX);
    bdcCalTree->SetBranchAddress("BDC1_ThetaY", &BDC1_ThetaY);
    bdcCalTree->SetBranchAddress("BDC1_X", &BDC1_X);
    bdcCalTree->SetBranchAddress("BDC1_Y", &BDC1_Y);
    bdcCalTree->SetBranchAddress("BDC2_ThetaX", &BDC2_ThetaX);
    bdcCalTree->SetBranchAddress("BDC2_ThetaY", &BDC2_ThetaY);
    bdcCalTree->SetBranchAddress("BDC2_X", &BDC2_X);
    bdcCalTree->SetBranchAddress("BDC2_Y", &BDC2_Y);

    bdcRawTree->SetBranchAddress("bdc1_drift", &bdc1_drift);
  }

  ~GetterBDCBase() {
    bdcInputFile->Close();
    delete bdcInputFile;
  }

  Long64_t GetBDCEntries() { return bdcCalTree->GetEntries(); }

  void GetBDCEntry(Long64_t entry) {
    bdcCalTree->GetEntry(entry);
    bdcRawTree->GetEntry(entry);
  }

  Double_t GetBDC1ThetaX() { return BDC1_ThetaX; }
  Double_t GetBDC1ThetaY() { return BDC1_ThetaY; }
  Double_t GetBDC1X() { return BDC1_X; }
  Double_t GetBDC1Y() { return BDC1_Y; }
  Double_t GetBDC2ThetaX() { return BDC2_ThetaX; }
  Double_t GetBDC2ThetaY() { return BDC2_ThetaY; }
  Double_t GetBDC2X() { return BDC2_X; }
  Double_t GetBDC2Y() { return BDC2_Y; }

  std::vector<double> *Getbdc1drift() { return bdc1_drift; }
};

// RDC 基类：负责打开 ESPRI 文件、连接分支和提供 RDC Getter。
class GetterRDCBase {
private:
  TFile *rdcInputFile;
  TTree *rdcCalTree;

  Double_t rdcA[2];
  Double_t rdcB[2];
  Double_t rdcX[2];
  Double_t rdcY[2];

public:
  GetterRDCBase(const char *rdcFileName) {
    rdcInputFile = TFile::Open(rdcFileName, "READ");
    rdcCalTree =
        static_cast<TTree *>(rdcInputFile->Get("CalTreeESPRI"));

    rdcCalTree->SetBranchAddress("rdcA", rdcA);
    rdcCalTree->SetBranchAddress("rdcB", rdcB);
    rdcCalTree->SetBranchAddress("rdcX", rdcX);
    rdcCalTree->SetBranchAddress("rdcY", rdcY);
  }

  ~GetterRDCBase() {
    rdcInputFile->Close();
    delete rdcInputFile;
  }

  Long64_t GetRDCEntries() { return rdcCalTree->GetEntries(); }

  void GetRDCEntry(Long64_t entry) { rdcCalTree->GetEntry(entry); }

  Double_t GetrdcA(Int_t side) { return rdcA[side]; }
  Double_t GetrdcB(Int_t side) { return rdcB[side]; }
  Double_t GetrdcX(Int_t side) { return rdcX[side]; }
  Double_t GetrdcY(Int_t side) { return rdcY[side]; }

  Double_t GetLeftrdcA() { return rdcA[0]; }
  Double_t GetRightrdcA() { return rdcA[1]; }
};

// 派生类：同时继承 BDC 基类和 RDC 基类
//构造函数通过初始化列表调用两个基类的构造函数
//派生类GetterBDCRDCReader增加了一个同时接收两个文件的构造函数GetterBDCRDCReader(const char *bdcFileName, const char *rdcFileName)
class GetterBDCRDCReader : public GetterBDCBase, public GetterRDCBase {
public:
  GetterBDCRDCReader(const char *bdcFileName, const char *rdcFileName)
      : GetterBDCBase(bdcFileName), GetterRDCBase(rdcFileName) {}

  ~GetterBDCRDCReader() {}

  Long64_t GetEntries() { return GetBDCEntries(); }

  void GetEntry(Long64_t entry) {
    GetBDCEntry(entry);
    GetRDCEntry(entry);
  }
};

#endif
