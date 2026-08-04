#ifndef GETTERRDC_H
#define GETTERRDC_H

#include <RtypesCore.h>
#include <TFile.h>
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
  GetterRDCReader(const char *fileName) {
    inputFile = TFile::Open(fileName, "READ");
    calTree = static_cast<TTree *>(inputFile->Get("CalTreeESPRI"));

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
