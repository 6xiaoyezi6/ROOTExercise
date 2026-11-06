#ifndef CUTEVENT_H
#define CUTEVENT_H

#include "TClusterGAGGData.h"
#include "TCutG.h"
#include "TFile.h"
#include "TList.h"
#include "TROOT.h"
#include "TString.h"
#include "TTree.h"
#include <iostream>

class ReadEvent {
private:
  TFile *InputFile = nullptr;
  TTree *InputTree = nullptr;
  TClusterGAGGData *cg = nullptr;

public:
  ReadEvent(TString InputRootPath, TString InputTreeName)
  {
    InputFile = TFile::Open(InputRootPath.Data(), "READ");
    InputTree = (TTree*)InputFile->Get(InputTreeName.Data());
    InputTree->SetBranchAddress("ClusterGAGG", &cg);
  }

  ~ReadEvent()
  {
    delete cg;
    delete InputFile;
  }

  Long64_t GetInputEntries() { return InputTree->GetEntries(); }
  void GetInputEntry(Long64_t entry) { InputTree->GetEntry(entry); }
  TClusterGAGGData *GetClusterGAGG() { return cg; }
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
private:
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

public:
  Gate(TString CutPath, ReadEvent &InputReader)
  {
    reader = &InputReader;
    LoadCut(CutPath);
    GetCut();
  }

  Bool_t IsAlpha() {
    TClusterGAGGData *cg = reader->GetClusterGAGG();
    return AlphaCut->IsInside(cg->GetDeltaEGAGGEnergy(0), cg->GetClusterGAGGEnergy(0));
  }
};

#endif
