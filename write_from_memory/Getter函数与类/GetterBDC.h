#ifndef GETTERBDC_H
#define GETTERBDC_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

#include <vector>

/*
class GetterBDCReader
定义的类名是 GetterBDCReader，而：
GetterBDCReader(const char *fileName)
是这个类的构造函数。C++规定构造函数名称必须与类名完全一致，而且没有返回类型。
析构函数也必须使用相同类名，并在前面加 ~：
~GetterBDCReader()
完整关系如下：

class GetterBDCReader {
public:
  // 构造函数：名称必须和类名相同
  GetterBDCReader(const char *fileName) {
  }

  // 析构函数：~ 加类名
  ~GetterBDCReader() {
  }
};

*/

//这里里面的Getter函数是没有参数的
////“声明指针”和“创建对象并把对象地址赋给指针”是放在一个文件中的
class GetterBDCReader {
private:
 //这三个量只能定义在构造函数外面
 //inputFile、calTree 和 rawTree 如果定义在构造函数内部，它们只是局部变量，构造函数结束后这些变量名就不存在了
 //但构造函数之后的函数也要使用这些量
  TFile *inputFile;
  TTree *calTree;
  TTree *rawTree;

  Double_t BDC1_ThetaX;
  Double_t BDC1_ThetaY;
  Double_t BDC1_X;
  Double_t BDC1_Y;
  Double_t BDC2_ThetaX;
  Double_t BDC2_ThetaY;
  Double_t BDC2_X;
  Double_t BDC2_Y;

  std::vector<double> *bdc1_drift;
/////////////////////////////////////////上面这些量是成员变量
//成员变量就是“属于某个类对象的变量”。它们写在类里面、函数外面，用来保存这个对象需要长期使用的数据
//写在函数内部的是局部变量，函数结束，这些局域变量也就没有了
public:
  GetterBDCReader(const char *fileName) 
  {
    inputFile = TFile::Open(fileName, "READ");
    calTree = static_cast<TTree *>(inputFile->Get("CalTreeBDC"));
    rawTree = static_cast<TTree *>(inputFile->Get("RawBDCTree"));

    bdc1_drift = nullptr;

    calTree->SetBranchAddress("BDC1_ThetaX", &BDC1_ThetaX);
    calTree->SetBranchAddress("BDC1_ThetaY", &BDC1_ThetaY);
    calTree->SetBranchAddress("BDC1_X", &BDC1_X);
    calTree->SetBranchAddress("BDC1_Y", &BDC1_Y);
    calTree->SetBranchAddress("BDC2_ThetaX", &BDC2_ThetaX);
    calTree->SetBranchAddress("BDC2_ThetaY", &BDC2_ThetaY);
    calTree->SetBranchAddress("BDC2_X", &BDC2_X);
    calTree->SetBranchAddress("BDC2_Y", &BDC2_Y);

    rawTree->SetBranchAddress("bdc1_drift", &bdc1_drift);
  }

  ~GetterBDCReader() 
  {
    inputFile->Close();
    delete inputFile;
  }

  Long64_t GetEntries() { return calTree->GetEntries(); }

  void GetEntry(Long64_t entry) 
  {
    calTree->GetEntry(entry);
    rawTree->GetEntry(entry);
  }

  Double_t GetBDC1ThetaX() { return BDC1_ThetaX; }
  Double_t GetBDC1ThetaY() { return BDC1_ThetaY; }
  Double_t GetBDC1X() { return BDC1_X; }
  Double_t GetBDC1Y() { return BDC1_Y; }
  Double_t GetBDC2ThetaX() { return BDC2_ThetaX; }
  Double_t GetBDC2ThetaY() { return BDC2_ThetaY; }
  Double_t GetBDC2X() { return BDC2_X; }
  Double_t GetBDC2Y() { return BDC2_Y; }

  std::vector<double> *Getbdc1drift() { return bdc1_drift; }
};

#endif
