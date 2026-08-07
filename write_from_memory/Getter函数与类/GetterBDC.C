#include "GetterBDC.h"

#include <TCanvas.h>
#include <TH1.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>

void SetHistAxisTitle(TH1 *hist, const char *xTitle, const char *xUnit) {
  Double_t binWidth = hist->GetXaxis()->GetBinWidth(1);

  hist->GetXaxis()->SetTitle(Form("%s (%s)", xTitle, xUnit));
  hist->GetYaxis()->SetTitle(Form("Counts / %.3g %s", binWidth, xUnit));
}

void GetterBDC() {
  gStyle->SetOptStat(111111111);

/*
等号坐标是声明指针，等号右边是创建类对象并调用构造函数
GetterBDCReader 是类名/类型；reader 是指针变量；new GetterBDCReader(...) 创建了真正的类对象，并把对象地址交给 reader
GetterBDCReader   *       reader    = new GetterBDCReader(...);
        │           │         │                  │
        │           │         │                  └─ 创建类对象并调用构造函数
        │           │         └─ 声明一个名为 reader 的指针，它只能指向 GetterBDCReader 类型的对象
        │           └─ 表示 reader 是指针
        └─ 类名，同时也是一种数据类型（类似Int,Double）
*/

  //“声明指针”和“创建对象并把对象地址赋给指针”是放在一个文件中的，也可以分开到两个文件中(头文件中声明指针，源文件中创建对象并把对象地址赋给指针)
  GetterBDCReader *reader = new GetterBDCReader("run0291_BDC.root");

  gROOT->cd();

  TH1D *hBDC1ThetaX =new TH1D("hBDC1ThetaX", "BDC1 ThetaX", 200, -0.2, 0.2);
  SetHistAxisTitle(hBDC1ThetaX, "BDC1 ThetaX", "rad");

  const Double_t invalidValue = -999.0;
  Long64_t numberOfEntries = reader->GetEntries();

  for (Long64_t entry = 0; entry < numberOfEntries; entry++) {
    reader->GetEntry(entry);

    Double_t bdc1ThetaX = reader->GetBDC1ThetaX();
    if (bdc1ThetaX != invalidValue) {
      hBDC1ThetaX->Fill(bdc1ThetaX);
    }
  }

  TCanvas *canvas = new TCanvas("canvas", "BDC1 ThetaX", 900, 700);
  hBDC1ThetaX->Draw();
  canvas->Modified();
  canvas->Update();

  delete reader;
}
