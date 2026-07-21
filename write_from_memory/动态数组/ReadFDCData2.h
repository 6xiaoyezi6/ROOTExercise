#ifndef READFDCDATA2_H
#define READFDCDATA2_H

#include "RtypesCore.h"

struct FDC0Hit {
  Int_t EventID;
  Int_t LayerID;
  Int_t HitIndex;//HitIndex
  Int_t WireID;
  Double_t TDC;
};

#endif

