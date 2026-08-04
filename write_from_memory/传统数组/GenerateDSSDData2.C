#include "DSSDHit.h"

#include <TFile.h>
#include <TInterpreter.h>
#include <TRandom3.h>
#include <TTree.h>

#include <vector>


/*
std::vector<DSSDHit>也是逐个hit的保存，每一个DSSD的前面或后面最多有5个击中，
  那一个事件的动态数组的长度最大为10，即10个hit，每一个hit又保存了对应的
  EventID，DSSDSide，DSSDFace，SideName，FaceName，DSSDChannel，ADC。

root [3] tree->Scan("DSSDRawHits.EventID:DSSDRawHits.SideName:DSSDRawHits.FaceName:DSSDRawHits.Channel:DSSDRawHits.ADC","","",1,5)
同一个事件中所有hit的EventID都等于该事件的TTree entry编号。

*/

void FillDSSDFaceHits(std::vector<DSSDHit> &DSSDRawHits,
                      Int_t EventID,
                      Int_t Side,
                      Int_t Face,
                      Int_t Multiplicity,
                      TRandom3 &random)
{
  const Int_t nChannels = 32;
  Bool_t usedChannels[nChannels] = {kFALSE};

  for (Int_t hitIndex = 0; hitIndex < Multiplicity; hitIndex++) {
    Int_t Channel = random.Integer(nChannels);
    while (usedChannels[Channel]) {
      Channel = random.Integer(nChannels);
    }
    usedChannels[Channel] = kTRUE;

    DSSDHit hit;
    hit.EventID = EventID;
    hit.Side = Side;
    hit.Face = Face;
    if (Side == 0) {
      hit.SideName = "Left";
    } else {
      hit.SideName = "Right";
    }
    if (Face == 0) {
      hit.FaceName = "Front";
    } else {
      hit.FaceName = "Back";
    }
    hit.Channel = Channel;
    hit.ADC = random.Gaus(1000.0, 10.0);
    DSSDRawHits.push_back(hit);
  }
}

void GenerateDSSDData2()
{
  const Int_t nEvents = 10000;
  const Int_t maxMultiplicity = 5;

  gInterpreter->GenerateDictionary("DSSDHit;std::vector<DSSDHit>",
                                   "DSSDHit.h;vector");

  TFile *outputFile = new TFile("DSSDData2.root", "RECREATE");
  TTree *tree = new TTree("tree", "DSSD raw hit data");

  std::vector<DSSDHit> DSSDRawHits;
  tree->Branch("DSSDRawHits", &DSSDRawHits);

  TRandom3 random(0);

  for (Int_t event = 0; event < nEvents; event++) {
    DSSDRawHits.clear();

    // Side = 0: Left, Side = 1: Right.
    Int_t Side = random.Integer(2);
    Int_t frontMultiplicity = random.Integer(maxMultiplicity) + 1;
    Int_t backMultiplicity = random.Integer(maxMultiplicity) + 1;

    FillDSSDFaceHits(DSSDRawHits, event, Side, 0, frontMultiplicity, random);
    FillDSSDFaceHits(DSSDRawHits, event, Side, 1, backMultiplicity, random);

    tree->Fill();
  }

  tree->Write();
  outputFile->Close();
}
