
#ifndef CREATE_OBJECT_TREE_INITIAL_H
#define CREATE_OBJECT_TREE_INITIAL_H

#include "TObject.h"
#include "TVector3.h"

// BDC1 的成员及初始化在本类中独立维护。
class BDC1 : public TObject {
public:
    Double_t Energy;
    TVector3 Position;
    Double_t Hits;

    BDC1() {Initialize();}

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        Energy = kInvalidValue;
        Hits =   kInvalidValue;
        GetPosition()->SetXYZ(kInvalidValue, kInvalidValue, kInvalidValue);

    }

    //这里是给Position这个量定义了一个Getter函数，当然也可以不定义
    //若不定义，后面用detector->bdc1->Position.SetXYZ(1.0, 2.0, 3.0);进行操作
    TVector3 *GetPosition() {return &Position;}
    Double_t GetBDC1Hits() {return Hits;}

    ClassDefOverride(BDC1, 1);
};

// BDC2 的成员及初始化在本类中独立维护。
class BDC2 : public TObject {
public:
    Double_t Energy;
    TVector3 Position;

    BDC2() {Initialize();}

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        Energy = kInvalidValue;
        GetPosition()->SetXYZ(kInvalidValue, kInvalidValue, kInvalidValue);
    }

    TVector3 *GetPosition() {return &Position;}

    ClassDefOverride(BDC2, 1);
};

ClassImp(BDC1);
ClassImp(BDC2);

#ifdef __ROOTCLING__
#pragma link C++ class BDC1+;
#pragma link C++ class BDC2+;
#endif

#endif
