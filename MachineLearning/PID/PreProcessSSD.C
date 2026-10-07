/*
 * 功能：预处理 PID.root 中 TOGAXSI 的 DeltaEGAGG–ClusterGAGG 粒子鉴别样本，写入 PreProcessSSD.root/tree。
 * 方法：读取 ClusterGAGG 和 ClusterSSD 逐径迹 hit，按同一 EventID、TrackID 汇总主 GAGG、
 *       左右 DeltaE GAGG 及 SSD X1/Y1/X2/Y2 四层能量；四层之和写入 ClusterSSDEnergy。
 *       保留 GAGG 两层符合且 SSD 四层完整的 proton、d、t、3He、4He 径迹，PID 为 0、1、2、3、4。
 * 注意事项：三项能量为 Double_t，单位 MeV；EventID、TrackID 和 PID 为 Int_t，标签来自模拟粒子真值。
 *           hit.Energy 是模拟写出的逐径迹能量份额，已包含探测器能量展宽，不是出靶动能。
 *           EventID + TrackID 唯一标识样本，可用于 MLP 与传统方法逐径迹匹配；
 *           同一径迹多个晶体的能量求和；同一事件可输出多行，训练/测试应按 EventID 划分。
 *           SSD 使用含模拟能量展宽的逐径迹 StripEnergy，同层多个 hit 累加，不混入其他径迹。
 *           SSD 缺少任一层或含无效能量时跳过该粒子；GAGG 两层筛选保持原定义。
 *           默认使用本目录的绝对输入/输出路径；输出以 RECREATE 写入，依赖本机 legacy NPTool 数据字典。
 */

R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterGAGG.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterSSD.dylib)

#include "TClusterGAGGData.h"
#include "TClusterSSDData.h"
#include "TFile.h"
#include "TTree.h"
#include <cmath>
#include <iostream>
#include <set>
#include <string>

void PreProcessSSD(
  const char* inputPath = "/Users/yemingxin/ROOT_Exercise/MachineLearning/PID/PID.root",
  const char* outputPath = "/Users/yemingxin/ROOT_Exercise/MachineLearning/PID/PreProcessSSD.root")
{
  TFile* f = TFile::Open(inputPath, "READ");
  TTree* inputTree = (TTree*)f->Get("SimulatedTree");

  TClusterGAGGData* clusterGAGG = nullptr;
  TClusterSSDData* clusterSSD = nullptr;
  inputTree->SetBranchAddress("ClusterGAGG", &clusterGAGG);
  inputTree->SetBranchAddress("ClusterSSD", &clusterSSD);

  TFile* outputFile = new TFile(outputPath, "RECREATE");
  TTree* outputTree = new TTree("tree", "TOGAXSI DeltaE-E PID truth samples; energy in MeV");
  Double_t DeltaEGAGGEnergy = 0;
  Double_t ClusterGAGGEnergy = 0;
  Double_t ClusterSSDEnergy = 0;
  Int_t EventID = -1;
  Int_t TrackID = -1;
  Int_t PID = -1;
  outputTree->Branch("DeltaEGAGGEnergy", &DeltaEGAGGEnergy, "DeltaEGAGGEnergy/D");
  outputTree->Branch("ClusterGAGGEnergy", &ClusterGAGGEnergy, "ClusterGAGGEnergy/D");
  outputTree->Branch("ClusterSSDEnergy", &ClusterSSDEnergy, "ClusterSSDEnergy/D");
  outputTree->Branch("EventID", &EventID, "EventID/I");
  outputTree->Branch("TrackID", &TrackID, "TrackID/I");
  outputTree->Branch("PID", &PID, "PID/I");

  const char* particleNames[5] = {"proton", "d", "t", "3He", "4He"};
  Long64_t classCounts[5] = {0, 0, 0, 0, 0};
  Long64_t entries = inputTree->GetEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
    inputTree->GetEntry(entry);
    std::set<Int_t> processedTracks;
    for (UInt_t i = 0; i < clusterGAGG->fClusterGAGGHits.size(); i++) {
      const TClusterGAGGHit& first = clusterGAGG->fClusterGAGGHits[i];
      if (first.ClusterGAGGType != "ClusterGAGG")
        continue;
      if (!processedTracks.insert(first.TrackID).second)
        continue;

      // 实际模拟字符串为 proton、deuteron、triton、3He、alpha。
      PID = -1;
      if (first.ParticleName == "proton" || first.ParticleName == "1H") PID = 0;
      else if (first.ParticleName == "deuteron" || first.ParticleName == "2H" || first.ParticleName == "d") PID = 1;
      else if (first.ParticleName == "triton" || first.ParticleName == "3H" || first.ParticleName == "t") PID = 2;
      else if (first.ParticleName == "3He" || first.ParticleName == "He3") PID = 3;
      else if (first.ParticleName == "alpha" || first.ParticleName == "4He") PID = 4;
      if (PID < 0)
        continue;

      EventID = first.EventID;
      TrackID = first.TrackID;
      DeltaEGAGGEnergy = 0;
      ClusterGAGGEnergy = 0;
      Bool_t valid = true;
      for (UInt_t j = 0; j < clusterGAGG->fClusterGAGGHits.size(); j++) {
        const TClusterGAGGHit& hit = clusterGAGG->fClusterGAGGHits[j];
        if (hit.EventID != EventID || hit.TrackID != first.TrackID)
          continue;
        if (hit.ClusterGAGGType != "ClusterGAGG" &&
            hit.ClusterGAGGType != "DeltaEGAGGLeft" &&
            hit.ClusterGAGGType != "DeltaEGAGGRight")
          continue;
        if (hit.ParticleName != first.ParticleName || !std::isfinite(hit.Energy) || hit.Energy < 0) {
          valid = false;
          break;
        }
        if (hit.ClusterGAGGType == "ClusterGAGG")
          ClusterGAGGEnergy += hit.Energy;
        else
          DeltaEGAGGEnergy += hit.Energy;
      }
      if (!valid || !std::isfinite(DeltaEGAGGEnergy) || !std::isfinite(ClusterGAGGEnergy) ||
          DeltaEGAGGEnergy <= 0 || ClusterGAGGEnergy <= 0)
        continue;

      ClusterSSDEnergy = 0;
      Double_t ssdLayerEnergy[4] = {0, 0, 0, 0};
      Bool_t ssdLayerHit[4] = {false, false, false, false};
      for (UInt_t j = 0; j < clusterSSD->fClusterSSDHits.size(); j++) {
        const TClusterSSDHit& hit = clusterSSD->fClusterSSDHits[j];
        if (hit.EventID != EventID || hit.TrackID != TrackID)
          continue;
        Int_t layer = -1;
        if (hit.ClusterSSDType == "X1") layer = 0;
        else if (hit.ClusterSSDType == "Y1") layer = 1;
        else if (hit.ClusterSSDType == "X2") layer = 2;
        else if (hit.ClusterSSDType == "Y2") layer = 3;
        if (layer < 0)
          continue;
        if (!std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0) {
          valid = false;
          break;
        }
        ssdLayerEnergy[layer] += hit.StripEnergy;
        ssdLayerHit[layer] = true;
      }
      if (!valid || !ssdLayerHit[0] || !ssdLayerHit[1] || !ssdLayerHit[2] || !ssdLayerHit[3])
        continue;
      for (Int_t layer = 0; layer < 4; layer++)
        ClusterSSDEnergy += ssdLayerEnergy[layer];
      if (!std::isfinite(ClusterSSDEnergy))
        continue;
      outputTree->Fill();
      classCounts[PID]++;
    }
  }

  std::cout << "Input events: " << entries << std::endl;
  for (Int_t i = 0; i < 5; i++)
    std::cout << "PID " << i << " (" << particleNames[i] << "): " << classCounts[i] << std::endl;
  std::cout << "Written samples: " << outputTree->GetEntries() << std::endl;
  outputFile->cd();
  outputTree->Write();
  outputFile->Close();
  f->Close();
  delete outputFile;
  delete f;
}
