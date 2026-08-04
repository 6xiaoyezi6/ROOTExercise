#include "DSSDHit.h"

#include <TFile.h>
#include <TInterpreter.h>
#include <TTree.h>

#include <vector>

//把二维数组转化为动态数组<结构体>
/*
DSSDRawQ[4][32]
├── 固定保存全部128个通道
├── 命中通道：ADC约为1000
└── 未命中通道：-999

DSSDRawHits
├── 只保存真正命中的通道
└── 不保存未命中通道，因此没有-999

例如某事件只有 6 个命中：

DSSDRawQ 固定有128个元素：
6个有效ADC + 122个-999

DSSDRawHits.size() == 6：
只包含6个有效hit

这正是动态数组的主要作用：只保存实际存在的 hit。若把 -999 也放入 vector，那么每个事件的 vector 长度仍会固
定为 128，就失去了动态稀疏存储的意义。


*/
void ConvertData()
{
  const Int_t nFaces = 4;
  const Int_t nChannels = 32;
  const Double_t noHitValue = -999.0;

  gInterpreter->GenerateDictionary("DSSDHit;std::vector<DSSDHit>",
                                   "DSSDHit.h;vector");

  TFile *inputFile = new TFile("DSSDData.root", "READ");
  TTree *inputTree = (TTree *)inputFile->Get("tree");

  Double_t DSSDRawQ[nFaces][nChannels];
  inputTree->SetBranchAddress("DSSDRawQ", DSSDRawQ);

  TFile *outputFile = new TFile("ConvertData.root", "RECREATE");
  TTree *outputTree = new TTree("tree", "DSSD raw array and hit vector");

  std::vector<DSSDHit> DSSDRawHits;
  outputTree->Branch("DSSDRawQ", DSSDRawQ, "DSSDRawQ[4][32]/D");
  outputTree->Branch("DSSDRawHits", &DSSDRawHits);

  Long64_t nEntries = inputTree->GetEntries();

  for (Long64_t entry = 0; entry < nEntries; entry++) {
    inputTree->GetEntry(entry);
    DSSDRawHits.clear();

    for (Int_t face = 0; face < nFaces; face++) {
      for (Int_t channel = 0; channel < nChannels; channel++) {
        if (DSSDRawQ[face][channel] == noHitValue) {
          continue;
        }

        DSSDHit hit;
        hit.EventID = (Int_t)entry;
        hit.Side = face / 2;
        hit.Face = face % 2;
        if (hit.Side == 0) {
          hit.SideName = "Left";
        } else {
          hit.SideName = "Right";
        }
        if (hit.Face == 0) {
          hit.FaceName = "Front";
        } else {
          hit.FaceName = "Back";
        }
        hit.Channel = channel;
        hit.ADC = DSSDRawQ[face][channel];
        DSSDRawHits.push_back(hit);
      }
    }

    outputTree->Fill();
  }

  outputTree->Write();
  outputFile->Close();
  inputFile->Close();
}
