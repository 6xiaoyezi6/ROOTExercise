#ifndef READFDCDATA2_H
#define READFDCDATA2_H

#include "RtypesCore.h"

struct FDC0Hit {
  Int_t eventID;
  Int_t layerID;
  Int_t hit;
  Int_t wireID;
  Double_t TDC;
};

#endif
