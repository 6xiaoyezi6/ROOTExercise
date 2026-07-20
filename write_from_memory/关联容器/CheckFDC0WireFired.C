#include <iostream>

#include <tuple>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#include "/Users/yemingxin/nptool/Projects/Espri/TestMacros/ReadFDC0XML.cc"
#include "/Users/yemingxin/nptool/Projects/Espri/TestMacros/ReadFDC0XML.h"
#include "TSamuraiFDC0Data.h"

void CheckFDC0WireFired() {

  ReadFDC0XML fdc0XML(
      "/Users/yemingxin/nptool/Projects/S034/db/SAMURAIFDC0_20200109.xml");

  TFile *file = TFile::Open(
      "/Users/yemingxin/nptool/Outputs/Simulation/Espri.root", "READ");
  TTree *tree = (TTree *)file->Get("SimulatedTree");

  TSamuraiFDC0Data *fdc0 = nullptr;
  tree->SetBranchAddress("SamuraiFDC0", &fdc0);

  Long64_t nPrint = tree->GetEntries();

  if (nPrint > 20)
    nPrint = 20;

  for (Long64_t event = 0; event < nPrint; event++) {
    tree->GetEntry(event);

    // 每个 tuple 保存三个数据：(layer, wire, edge)
    std::vector<std::tuple<int, int, int>> x1;

    unsigned int mult = fdc0->Mult();

    // 存入数据
    for (unsigned int i = 0; i < mult; i++) {
      int layer = fdc0->GetLayerNbr(i);
      int wire = fdc0->GetWireNbr(i);
      int edge = fdc0->GetEdge(i);

      x1.emplace_back(layer, wire, edge);
    }

    std::cout << "\nEvent " << event << ", fired wires = " << x1.size()
              << std::endl;

    // 读取并打印数据
    for (const auto &hit : x1) {
      int layer = std::get<0>(hit);
      int wire = std::get<1>(hit);
      int edge = std::get<2>(hit);

      const char *axisName = "y";

      if (layer == 0 || layer == 1 || layer == 4 || layer == 5) {
        axisName = "x";
      }

      std::cout << "layer = " << layer << ", wire = " << wire
                << ", z = " << fdc0XML.GetWireZ(layer, wire) << ", " << axisName
                << " = " << fdc0XML.GetWireXY(layer, wire)
                << ", edge = " << edge << std::endl;
    }
  }

  file->Close();
}
