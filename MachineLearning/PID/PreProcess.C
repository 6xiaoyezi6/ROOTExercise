/*
 * 功能：预处理 PID.root 中 TOGAXSI 的 DeltaEGAGG–ClusterGAGG 粒子鉴别样本，写入 PreProcess.root/tree。
 * 方法：读取 ClusterGAGG 逐径迹 hit，按同一 EventID、TrackID 汇总主 GAGG 和左右 DeltaE GAGG 的能量；
 *       仅保留两层能量均有限且大于 0 的 proton、d、t、3He、4He 径迹，PID 依次为 0、1、2、3、4。
 * 注意事项：两项能量为 Double_t，单位 MeV；EventID、TrackID 和 PID 为 Int_t，标签来自模拟粒子真值。
 *           hit.Energy 是模拟写出的逐径迹能量份额，已包含探测器能量展宽，不是出靶动能。
 *           EventID + TrackID 唯一标识样本，可用于 MLP 与传统方法逐径迹匹配；
 *           同一径迹多个晶体的能量求和；同一事件可输出多行，训练/测试应按 EventID 划分。
 *           不要求 SSD 命中，也不对不同粒子的晶体总能量混合求和；无两层符合的径迹不写入。
 *           默认使用本目录的绝对输入/输出路径；输出以 RECREATE 写入，依赖本机 legacy NPTool 数据字典。
 */

R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterGAGG.dylib)

#include "TClusterGAGGData.h"
#include "TFile.h"
#include "TTree.h"
#include <cmath>
#include <iostream>
#include <set>
#include <string>

void PreProcess(
  const char* inputPath = "/Users/yemingxin/ROOT_Exercise/MachineLearning/PID/PID.root",
  const char* outputPath = "/Users/yemingxin/ROOT_Exercise/MachineLearning/PID/PreProcess.root")
{
  TFile* f = TFile::Open(inputPath, "READ");
  TTree* inputTree = (TTree*)f->Get("SimulatedTree");

  TClusterGAGGData* clusterGAGG = nullptr;
  inputTree->SetBranchAddress("ClusterGAGG", &clusterGAGG);

  TFile* outputFile = new TFile(outputPath, "RECREATE");
  TTree* outputTree = new TTree("tree", "TOGAXSI DeltaE-E PID truth samples; energy in MeV");
  Double_t DeltaEGAGGEnergy = 0;
  Double_t ClusterGAGGEnergy = 0;
  Int_t EventID = -1;
  Int_t TrackID = -1;
  Int_t PID = -1;
  outputTree->Branch("DeltaEGAGGEnergy", &DeltaEGAGGEnergy, "DeltaEGAGGEnergy/D");
  outputTree->Branch("ClusterGAGGEnergy", &ClusterGAGGEnergy, "ClusterGAGGEnergy/D");
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
