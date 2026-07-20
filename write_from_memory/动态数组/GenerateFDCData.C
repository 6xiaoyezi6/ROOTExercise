#include <vector>

#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

void GenerateFDCData() {

  const Int_t numberOfLayers = 8;
  const Int_t numberOfWires = 32;
  const Double_t tdcMean = 1000.0;
  const Double_t tdcSigma = 100.0;
  const Double_t invalidTdcLayerProbability = 0.20;

  TFile *file = new TFile("FDCData.root", "RECREATE");
  TTree *tree = new TTree("tree", "Simulated FDC hit data");

  Int_t eventID = 0;

  // 这三个动态数组使用相同的 hit 索引。
  // FDC0_layerid[i]、FDC0_wireid[i] 和 FDC0_TDC[i]
  // 共同描述第 i 个 hit。
  std::vector<Int_t> FDC0_layerid;
  std::vector<Int_t> FDC0_wireid;
  std::vector<Double_t> FDC0_TDC;
  // 固定保存 8 个元素，第 layer 个元素表示该层的 hit 数量。
  std::vector<Int_t> FDC0_layer_nhits;

  tree->Branch("eventID", &eventID, "eventID/I");
  tree->Branch("FDC0_layerid", &FDC0_layerid);
  tree->Branch("FDC0_wireid", &FDC0_wireid);
  tree->Branch("FDC0_TDC", &FDC0_TDC);
  tree->Branch("FDC0_layer_nhits", &FDC0_layer_nhits);

  TRandom3 random(0);

  for (eventID = 0; eventID < 10000; eventID++) {
    FDC0_layerid.clear();
    FDC0_wireid.clear();
    FDC0_TDC.clear();
    FDC0_layer_nhits.assign(numberOfLayers, 0);

    Int_t invalidTdcLayer = -1;

    // 每个事件有 20% 的概率随机选择一层，将该层的 TDC 填成 -999。
    if (random.Uniform(0.0, 1.0) < invalidTdcLayerProbability) {
      invalidTdcLayer = random.Integer(numberOfLayers);//生成一个均匀分布的随机整数
    }

    for (Int_t layer = 0; layer < numberOfLayers; layer++) {
      // 每一层随机产生 1--32 个 hit。
      Int_t numberOfHitsInLayer = random.Integer(numberOfWires) + 1;
      FDC0_layer_nhits[layer] = numberOfHitsInLayer;

      // 防止同一层中的同一根丝被重复选中。
      Bool_t wireUsed[numberOfWires] = {kFALSE};

      for (Int_t hit = 0; hit < numberOfHitsInLayer; hit++) {
        Int_t wireID = random.Integer(numberOfWires);

        while (wireUsed[wireID]) {
          wireID = random.Integer(numberOfWires);
        }

        wireUsed[wireID] = kTRUE;

        // 正常层中的每一个 hit 都独立生成自己的高斯随机 TDC。
        // 被随机选中的异常层，其所有 hit 的 TDC 都填成 -999。
        Double_t tdc = random.Gaus(tdcMean, tdcSigma);
        if (layer == invalidTdcLayer) {
          tdc = -999.0;
        }

        FDC0_layerid.push_back(layer);
        FDC0_wireid.push_back(wireID);
        FDC0_TDC.push_back(tdc);
      }
    }

    tree->Fill();
  }

  file->cd();
  tree->Write();
  file->Close();
}
