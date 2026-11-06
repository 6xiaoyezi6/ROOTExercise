#include "TObject.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TVector3.h"
#include <iostream>

Double_t kInvalidValue = -999.0;

// 第一步：定义数据容器。一个对象存放一个事件中该探测器的数据。
class BDC : public TObject {
public:
    Double_t X;
    Double_t Y;
    Double_t Z;
    Double_t U;

    BDC() { Initialize(); }

    void Initialize()
    {
        X = kInvalidValue;
        Y = kInvalidValue;
        Z = kInvalidValue;
        U = kInvalidValue;
    }

    //为自定义类添加ROOT类型识别、运行时反射和对象持久化所需要的接口，并指定类的版本号
    ClassDefOverride(BDC, 1);
};

class RecoilGAGG : public TObject {
public:
    Double_t Energy;

    RecoilGAGG() { Initialize(); }

    void Initialize()
    {
        Energy = kInvalidValue;
    }

    ClassDefOverride(RecoilGAGG, 2);
};

class RecoilSSD : public TObject {
public:
    TVector3 Position;

    RecoilSSD() { Initialize(); }

    void Initialize()
    {
        Position.SetXYZ(kInvalidValue, kInvalidValue, kInvalidValue);
    }

    ClassDefOverride(RecoilSSD, 3);
};

ClassImp(BDC);
ClassImp(RecoilGAGG);
ClassImp(RecoilSSD);

// 将三个数据类交给 ROOT 字典生成器，+ 表示生成持久化所需的代码。
#ifdef __ROOTCLING__
#pragma link C++ class BDC+;
#pragma link C++ class RecoilGAGG+;
#pragma link C++ class RecoilSSD+;
#endif

