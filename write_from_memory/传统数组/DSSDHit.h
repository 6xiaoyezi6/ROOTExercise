#ifndef DSSD_HIT_H
#define DSSD_HIT_H

#include <RtypesCore.h>
#include <TString.h>

struct DSSDHit
{
  Int_t EventID;
  Int_t Side;     // 0: Left, 1: Right
  Int_t Face;     // 0: Front, 1: Back
  TString SideName; //不推荐用字符串
  TString FaceName; //不推荐用字符串
  Int_t Channel;  // 0-31
  Double_t ADC;
};

#endif
