#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1F.h"
#include "TMath.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TVector3.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/LeastSquares/LeastSquares.cc"
#include "/Users/yemingxin/nptool/Projects/Espri/TestMacros/ReadFDC0XML.h"
#include "/Users/yemingxin/nptool/Projects/Espri/TestMacros/ReadFDC0XML.cc"
#include "TSamuraiFDC0Data.h"

void CheckFDC0WireFired()
{
  const char* inputFileName = "/Users/yemingxin/nptool/Outputs/Simulation/Espri.root";

  //定义一个ReadFDC0XML类的对象，并在定义对象时调用它的构造函数
  ReadFDC0XML fdc0XML("/Users/yemingxin/nptool/Projects/S034/db/SAMURAIFDC0_20200109.xml");

  TFile* file = TFile::Open(inputFileName, "READ");
  TTree* tree = (TTree*)file->Get("SimulatedTree");


  TSamuraiFDC0Data* fdc0 = 0;
  tree->SetBranchAddress("SamuraiFDC0", &fdc0);

  Long64_t nEntries = tree->GetEntries();
  Long64_t nPrint = nEntries;
  if(nPrint > 20)
    nPrint = 20;


  for(Long64_t event = 0; event < nPrint; event++){
    tree->GetEntry(event);

    //先定义一个关联容器firedWires，它里面存放的元素类型是 std::pair<int, int>
    //然后把每一个事件的层号和丝号存入关联容器中
    //.insert() 和 std::make_pair() 都是 C++ 的标准用法
    std::set<std::pair<int, int> > firedWires;

    if(fdc0)
    {
      unsigned int mult = fdc0->Mult();
      for(unsigned int i = 0; i < mult; i++)
      {
        int layer = fdc0->GetLayerNbr(i);
        int wire = fdc0->GetWireNbr(i);
        firedWires.insert(std::make_pair(layer, wire));
      }
    }

    std::cout << std::endl;
    std::cout << "Event " << event << "  FDC0 fired wires = " << firedWires.size() << std::endl;
    std::cout << "  layer  wire       z(mm)       x/y(mm)" << std::endl;

    //把关联容器firedWires 中的数据取出来
    //.first 和 .second 是 C++ 标准库中 std::pair 的标准成员 .first是关联容器的第一个值 .second是关联容器的第二个值
    for(const auto& hit : firedWires)
    {
    int layer = hit.first;
    int wire = hit.second;

    const char* axisName = "y";
    if(layer == 0 || layer == 1 || layer == 4 || layer == 5)
        axisName = "x";

    std::cout << "layer = " << layer
          << ", wire = " << wire
          << ", z = " << fdc0XML.GetWireZ(layer, wire)
          << ", " << axisName << " = " << fdc0XML.GetWireXY(layer, wire)
          << std::endl;
    }
  }

  file->Close();
}
