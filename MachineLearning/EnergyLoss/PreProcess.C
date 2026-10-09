/*
 * 功能：依次从 alphap.root 和 alphap_test.root 的 SimulatedTree 筛选前向 alpha 单击中事件，
 *       分别写入 Processd.root/tree 和 Processd_test.root/tree。
 * 方法：四层 ClusterSSD 各保留一个 hit，要求同侧、同一 TrackID，并匹配唯一主 ClusterGAGG hit；
 *       汇总同一 alpha 轨迹的 DeltaE GAGG 能量，提取真实 SSD 世界坐标、顶点和顶点总动能。
 * 注意事项：能量为 MeV，坐标为 mm；位置与标签均为模拟真值。排除多击中和无效值。
 *           主 GAGG 与 DeltaE GAGG 能量分别保存；无匹配 DeltaE hit 时 DeltaEGAGGEnergy 为 0。
 *           DeltaE 可有多个匹配 hit，其能量求和；不额外要求 DeltaE 单击中。
 *           在 EnergyLoss 目录运行；每份输入使用独立的树和计数器，输出文件以 RECREATE 写入。
 *           依赖本机 legacy NPTool 数据字典。
 */

R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterSSD.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterGAGG.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPReactionConditions.dylib)

#include "TClusterSSDData.h"
#include "TClusterGAGGData.h"
#include "TReactionConditions.h"
#include "TFile.h"
#include "TTree.h"
#include <cmath>
#include <iostream>

static void ProcessEnergyLossFile(const char* inputPath, const char* outputPath)
{
  const Double_t kInvalidValue = -999.0;

  std::cout << "Processing: " << inputPath << " -> " << outputPath << std::endl;
  TFile* f = TFile::Open(inputPath, "READ");
  TTree* inputTree = (TTree*)f->Get("SimulatedTree");

  TClusterSSDData* clusterSSD = nullptr;
  TClusterGAGGData* clusterGAGG = nullptr;
  TReactionConditions* reaction = nullptr;

  inputTree->SetBranchAddress("ClusterSSD", &clusterSSD);
  inputTree->SetBranchAddress("ClusterGAGG", &clusterGAGG);
  inputTree->SetBranchAddress("ReactionConditions", &reaction);

  TFile* outputFile = new TFile(outputPath, "RECREATE");
  TTree* outputTree = new TTree("tree", "Forward alpha single-hit truth samples; energy MeV, position mm");
  Double_t ClusterGAGGEnergy, DeltaEGAGGEnergy;
  Double_t ClusterSSDX1, ClusterSSDY1, ClusterSSDX2, ClusterSSDY2;
  Double_t ClusterSSDZX1, ClusterSSDZY1, ClusterSSDZX2, ClusterSSDZY2;
  Double_t VertexX, VertexY, VertexZ, KinematicsAtVertex;

  outputTree->Branch("ClusterGAGGEnergy", &ClusterGAGGEnergy, "ClusterGAGGEnergy/D");
  outputTree->Branch("DeltaEGAGGEnergy", &DeltaEGAGGEnergy, "DeltaEGAGGEnergy/D");
  outputTree->Branch("ClusterSSDX1", &ClusterSSDX1, "ClusterSSDX1/D");
  outputTree->Branch("ClusterSSDY1", &ClusterSSDY1, "ClusterSSDY1/D");
  outputTree->Branch("ClusterSSDX2", &ClusterSSDX2, "ClusterSSDX2/D");
  outputTree->Branch("ClusterSSDY2", &ClusterSSDY2, "ClusterSSDY2/D");
  outputTree->Branch("ClusterSSDZX1", &ClusterSSDZX1, "ClusterSSDZX1/D");
  outputTree->Branch("ClusterSSDZY1", &ClusterSSDZY1, "ClusterSSDZY1/D");
  outputTree->Branch("ClusterSSDZX2", &ClusterSSDZX2, "ClusterSSDZX2/D");
  outputTree->Branch("ClusterSSDZY2", &ClusterSSDZY2, "ClusterSSDZY2/D");
  outputTree->Branch("VertexX", &VertexX, "VertexX/D");
  outputTree->Branch("VertexY", &VertexY, "VertexY/D");
  outputTree->Branch("VertexZ", &VertexZ, "VertexZ/D");
  outputTree->Branch("KinematicsAtVertex", &KinematicsAtVertex, "KinematicsAtVertex/D");

  Long64_t entries = inputTree->GetEntries();
  Long64_t singleSSD = 0;
  Long64_t matchedGAGG = 0;
  for (Long64_t event = 0; event < entries; event++) {
    inputTree->GetEntry(event);

    // 严格单击中：整个前向 SSD 中恰有四个 hit，不忽略其他粒子或其他侧的 hit。
    if (clusterSSD->fClusterSSDHits.size() != 4)
      continue;

    Int_t hitIndex[4] = {-1, -1, -1, -1};
    Bool_t valid = true;
    const TClusterSSDHit& first = clusterSSD->fClusterSSDHits[0];
    if (first.ClusterSSDGroup != "Left" && first.ClusterSSDGroup != "Right")
      continue;
    for (Int_t i = 0; i < 4; i++) {
      const TClusterSSDHit& hit = clusterSSD->fClusterSSDHits[i];
      if (hit.ParticleName != "alpha" || hit.TrackID != first.TrackID ||
          hit.ClusterSSDGroup != first.ClusterSSDGroup ||
          !std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0) {
        valid = false;
        break;
      }
      Int_t layer = -1;
      if (hit.ClusterSSDType == "X1") layer = 0;
      else if (hit.ClusterSSDType == "Y1") layer = 1;
      else if (hit.ClusterSSDType == "X2") layer = 2;
      else if (hit.ClusterSSDType == "Y2") layer = 3;
      if (layer < 0 || hitIndex[layer] >= 0) {
        valid = false;
        break;
      }
      hitIndex[layer] = i;
    }
    if (!valid)
      continue;
    singleSSD++;

    // DeltaE hit 不计入主 GAGG 的单击中数，能量保存到独立分支。
    Int_t mainHitIndex = -1;
    Int_t mainHitCount = 0;
    for (UInt_t i = 0; i < clusterGAGG->fClusterGAGGHits.size(); i++) {
      if (clusterGAGG->fClusterGAGGHits[i].ClusterGAGGType == "ClusterGAGG") {
        mainHitIndex = i;
        mainHitCount++;
      }
    }
    if (mainHitCount != 1 || clusterGAGG->GetClusterGAGGMultEnergy() != 1)
      continue;
    const TClusterGAGGHit& gag = clusterGAGG->fClusterGAGGHits[mainHitIndex];
    if (gag.ParticleName != "alpha" || gag.TrackID != first.TrackID ||
        gag.CrystalNbr != clusterGAGG->GetClusterGAGGCristalNbr(0))
      continue;
    matchedGAGG++;

    DeltaEGAGGEnergy = 0;
    for (UInt_t i = 0; i < clusterGAGG->fClusterGAGGHits.size(); i++) {
      const TClusterGAGGHit& hit = clusterGAGG->fClusterGAGGHits[i];
      if ((hit.ClusterGAGGType == "DeltaEGAGGLeft" ||
           hit.ClusterGAGGType == "DeltaEGAGGRight") &&
          hit.ParticleName == "alpha" && hit.TrackID == first.TrackID) {
        if (!std::isfinite(hit.Energy) || hit.Energy < 0) {
          valid = false;
          break;
        }
        DeltaEGAGGEnergy += hit.Energy;
      }
    }
    if (!valid)
      continue;

    Int_t alphaIndex = -1;
    Int_t alphaCount = 0;
    for (Int_t i = 0; i < reaction->GetParticleMultiplicity(); i++) {
      if (reaction->GetParticleName(i) == "alpha") {
        alphaIndex = i;
        alphaCount++;
      }
    }
    if (alphaCount != 1)
      continue;

    const TVector3& x1 = clusterSSD->fClusterSSDHits[hitIndex[0]].InteractionPositionWorld;
    const TVector3& y1 = clusterSSD->fClusterSSDHits[hitIndex[1]].InteractionPositionWorld;
    const TVector3& x2 = clusterSSD->fClusterSSDHits[hitIndex[2]].InteractionPositionWorld;
    const TVector3& y2 = clusterSSD->fClusterSSDHits[hitIndex[3]].InteractionPositionWorld;
    ClusterGAGGEnergy = clusterGAGG->GetClusterGAGGEnergy(0);
    ClusterSSDX1 = x1.X();
    ClusterSSDY1 = y1.Y();
    ClusterSSDX2 = x2.X();
    ClusterSSDY2 = y2.Y();
    ClusterSSDZX1 = x1.Z();
    ClusterSSDZY1 = y1.Z();
    ClusterSSDZX2 = x2.Z();
    ClusterSSDZY2 = y2.Z();
    VertexX = reaction->GetVertexPositionX();
    VertexY = reaction->GetVertexPositionY();
    VertexZ = reaction->GetVertexPositionZ();
    KinematicsAtVertex = reaction->GetKineticEnergy(alphaIndex);

    Double_t values[14] = {ClusterGAGGEnergy, DeltaEGAGGEnergy, ClusterSSDX1, ClusterSSDY1,
      ClusterSSDX2, ClusterSSDY2, ClusterSSDZX1, ClusterSSDZY1,
      ClusterSSDZX2, ClusterSSDZY2, VertexX, VertexY, VertexZ, KinematicsAtVertex};
    for (Int_t i = 0; i < 14; i++) {
      if (!std::isfinite(values[i]) || values[i] == kInvalidValue)
        valid = false;
    }
    if (!valid || ClusterGAGGEnergy <= 0 || KinematicsAtVertex <= 0)
      continue;
    outputTree->Fill();
  }

  std::cout << "Input events: " << entries << std::endl;
  std::cout << "Four-layer same-track alpha single-hit events: " << singleSSD << std::endl;
  std::cout << "Matched single main-GAGG events: " << matchedGAGG << std::endl;
  std::cout << "Written valid events: " << outputTree->GetEntries() << std::endl;
  outputFile->cd();
  outputTree->Write();
  outputFile->Close();
  f->Close();
  delete outputFile;
  delete f;
}

void PreProcess()
{
  ProcessEnergyLossFile("alphap.root", "Processd.root");
  ProcessEnergyLossFile("alphap_test.root", "Processd_test.root");
}
