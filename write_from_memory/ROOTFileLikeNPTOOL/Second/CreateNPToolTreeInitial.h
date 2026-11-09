/*
 * 功能：集中定义对象分支的数据类 BDC1、BDC2，分别保存 Energy、Position。
 * 方法：各类通过 Initialize 将能量及坐标三个分量设为 -999，并分别声明 ROOT
 *       类型信息及持久化字典，供创建器和读取器共用。
 * 注意事项：新增分支数据类时，在本文件追加类、ClassImp 和 pragma link 声明，
 *           并在创建器和读取器中绑定新分支。Position 内嵌为 TVector3，
 *           通过 GetPosition 返回指针访问；使用 ACLiC 生成数据类字典。
 */
#ifndef SECOND_CREATE_NPTOOL_TREE_INITIAL_H
#define SECOND_CREATE_NPTOOL_TREE_INITIAL_H

#include "TObject.h"
#include "TVector3.h"

// BDC1 的成员及初始化在本类中独立维护。
class BDC1 : public TObject {
public:
    Double_t Energy;
    TVector3 Position;
    //Double_t Hits;

    BDC1() {Initialize();}

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        Energy = kInvalidValue;
        //Hits =   kInvalidValue;
        GetPosition()->SetXYZ(kInvalidValue, kInvalidValue, kInvalidValue);

    }

    //这里是给Position这个量定义了一个Getter函数，当然也可以不定义
    //若不定义，后面用detector->bdc1->Position.SetXYZ(1.0, 2.0, 3.0);进行操作
    TVector3 *GetPosition() {return &Position;}

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
