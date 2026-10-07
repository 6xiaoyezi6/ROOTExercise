/*
 * 功能：从 alpha-proton 弹性散射模拟中筛选匹配的七层 SSD 条带单击中事件，
 *       顺序生成 Vertex/Processd.root 和 Processd_test.root，各含 tree 的 14 个位置标量分支及 Long64_t EventID。
 * 方法：Cluster 四层取同侧同轨迹 alpha，Recoil 三层取同组同轨迹 proton；
 *       Left 仅配 RU/RD，Right 仅配 LU/LD。Cluster 使用条带世界 X/Y/Z，
 *       Recoil 使用条带局域 X/X/Y；ReactionConditions 的真实世界顶点为训练标签。
 * 注意事项：所有位置单位 mm；每个原始事件最多输出一行，多击中、错配和无效值被排除。
 *           TrackID 只在各自粒子的层之间匹配，alpha 与 proton 不要求同一 TrackID。
 *           不使用 GAGG 或传统顶点重建成功条件；不使用真实击中坐标作为输入。
 *           输入默认复用 ../EnergyLoss/alphap.root 与 alphap_test.root，输出仅写本宏目录。
 *           EventID 取 SSD hit 的模拟事件号，在各输入文件内唯一；两文件的 ID 不混用。
 *           输出使用 RECREATE；依赖本机 legacy NPTool 数据字典；本文件交付时未编译运行。
 */
R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterSSD.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPRecoilSSD.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPReactionConditions.dylib)

#include "TClusterSSDData.h"
#include "TRecoilSSDData.h"
#include "TReactionConditions.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>

static void ProcessVertexFile(const char* inputPath, const char* outputPath)
{
  const Double_t kInvalidValue = -999.0;
  std::cout << "Processing: " << inputPath << " -> " << outputPath << std::endl;
  TFile* f = TFile::Open(inputPath, "READ");
  TTree* inputTree = (TTree*)f->Get("SimulatedTree");

  TClusterSSDData* clusterSSD = nullptr;
  TRecoilSSDData* recoilSSD = nullptr;
  TReactionConditions* reaction = nullptr;
  inputTree->SetBranchAddress("ClusterSSD", &clusterSSD);
  inputTree->SetBranchAddress("RecoilSSD", &recoilSSD);
  inputTree->SetBranchAddress("ReactionConditions", &reaction);

  TFile* outputFile = new TFile(outputPath, "RECREATE");
  TTree* outputTree = new TTree("tree", "Matched alpha-proton SSD strip coordinates and true world vertex; mm");
  Double_t ClusterSSDX1, ClusterSSDZX1, ClusterSSDY1, ClusterSSDZY1;
  Double_t ClusterSSDX2, ClusterSSDZX2, ClusterSSDY2, ClusterSSDZY2;
  Double_t RecoilSSDX1Locus, RecoilSSDX2Locus, RecoilSSDY1Locus;
  Double_t VertexX, VertexY, VertexZ;
  Long64_t EventID = -1;
  std::set<Long64_t> writtenEventIDs;
  outputTree->Branch("EventID", &EventID, "EventID/L");

  outputTree->Branch("ClusterSSDX1", &ClusterSSDX1, "ClusterSSDX1/D");
  outputTree->Branch("ClusterSSDZX1", &ClusterSSDZX1, "ClusterSSDZX1/D");
  outputTree->Branch("ClusterSSDY1", &ClusterSSDY1, "ClusterSSDY1/D");
  outputTree->Branch("ClusterSSDZY1", &ClusterSSDZY1, "ClusterSSDZY1/D");
  outputTree->Branch("ClusterSSDX2", &ClusterSSDX2, "ClusterSSDX2/D");
  outputTree->Branch("ClusterSSDZX2", &ClusterSSDZX2, "ClusterSSDZX2/D");
  outputTree->Branch("ClusterSSDY2", &ClusterSSDY2, "ClusterSSDY2/D");
  outputTree->Branch("ClusterSSDZY2", &ClusterSSDZY2, "ClusterSSDZY2/D");
  outputTree->Branch("RecoilSSDX1Locus", &RecoilSSDX1Locus, "RecoilSSDX1Locus/D");
  outputTree->Branch("RecoilSSDX2Locus", &RecoilSSDX2Locus, "RecoilSSDX2Locus/D");
  outputTree->Branch("RecoilSSDY1Locus", &RecoilSSDY1Locus, "RecoilSSDY1Locus/D");
  outputTree->Branch("VertexX", &VertexX, "VertexX/D");
  outputTree->Branch("VertexY", &VertexY, "VertexY/D");
  outputTree->Branch("VertexZ", &VertexZ, "VertexZ/D");

  Long64_t entries = inputTree->GetEntries();
  Long64_t singleCluster = 0, singleRecoil = 0, matchedPairs = 0;
  for (Long64_t event = 0; event < entries; event++) {
    inputTree->GetEntry(event);
    // 两种 SSD 分别严格要求四个和三个 hit；不任意取多击中中的第一个。
    if (clusterSSD->fClusterSSDHits.size() != 4)
      continue;
    const TClusterSSDHit& firstCluster = clusterSSD->fClusterSSDHits[0];
    if (firstCluster.ClusterSSDGroup != "Left" && firstCluster.ClusterSSDGroup != "Right")
      continue;
    Int_t clusterIndex[4] = {-1, -1, -1, -1};
    Bool_t valid = true;
    for (Int_t i = 0; i < 4; i++) {
      const TClusterSSDHit& hit = clusterSSD->fClusterSSDHits[i];
      if (hit.ParticleName != "alpha" || hit.TrackID != firstCluster.TrackID ||
          hit.EventID != firstCluster.EventID || hit.ClusterSSDGroup != firstCluster.ClusterSSDGroup ||
          !std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0.0) {
        valid = false;
        break;
      }
      Int_t layer = -1;
      if (hit.ClusterSSDType == "X1") layer = 0;
      else if (hit.ClusterSSDType == "Y1") layer = 1;
      else if (hit.ClusterSSDType == "X2") layer = 2;
      else if (hit.ClusterSSDType == "Y2") layer = 3;
      if (layer < 0 || clusterIndex[layer] >= 0) {
        valid = false;
        break;
      }
      clusterIndex[layer] = i;
    }
    if (!valid)
      continue;
    singleCluster++;

    if (recoilSSD->fRecoilSSDHits.size() != 3)
      continue;
    const TRecoilSSDHit& firstRecoil = recoilSSD->fRecoilSSDHits[0];
    Int_t recoilIndex[3] = {-1, -1, -1};
    for (Int_t i = 0; i < 3; i++) {
      const TRecoilSSDHit& hit = recoilSSD->fRecoilSSDHits[i];
      if (hit.ParticleName != "proton" || hit.TrackID != firstRecoil.TrackID ||
          hit.EventID != firstCluster.EventID || hit.RecoilSSDGroup != firstRecoil.RecoilSSDGroup ||
          !std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0.0) {
        valid = false;
        break;
      }
      Int_t layer = -1;
      if (hit.RecoilSSDType == "X1") layer = 0;
      else if (hit.RecoilSSDType == "X2") layer = 1;
      else if (hit.RecoilSSDType == "Y1") layer = 2;
      if (layer < 0 || recoilIndex[layer] >= 0) {
        valid = false;
        break;
      }
      recoilIndex[layer] = i;
    }
    if (!valid)
      continue;
    singleRecoil++;

    Bool_t pairOK = false;
    if (firstCluster.ClusterSSDGroup == "Left")
      pairOK = firstRecoil.RecoilSSDGroup == "RU" || firstRecoil.RecoilSSDGroup == "RD";
    else if (firstCluster.ClusterSSDGroup == "Right")
      pairOK = firstRecoil.RecoilSSDGroup == "LU" || firstRecoil.RecoilSSDGroup == "LD";
    if (!pairOK)
      continue;
    matchedPairs++;

    // 标签为真实反应顶点；粒子名核对用于确认 alpha-proton 两体样本。
    if (reaction->GetParticleMultiplicity() != 2)
      continue;
    Int_t alphaCount = 0, protonCount = 0;
    for (Int_t i = 0; i < reaction->GetParticleMultiplicity(); i++) {
      if (reaction->GetParticleName(i) == "alpha") alphaCount++;
      if (reaction->GetParticleName(i) == "proton") protonCount++;
    }
    if (alphaCount != 1 || protonCount != 1)
      continue;

    const TClusterSSDHit& cx1 = clusterSSD->fClusterSSDHits[clusterIndex[0]];
    const TClusterSSDHit& cy1 = clusterSSD->fClusterSSDHits[clusterIndex[1]];
    const TClusterSSDHit& cx2 = clusterSSD->fClusterSSDHits[clusterIndex[2]];
    const TClusterSSDHit& cy2 = clusterSSD->fClusterSSDHits[clusterIndex[3]];
    const TRecoilSSDHit& rx1 = recoilSSD->fRecoilSSDHits[recoilIndex[0]];
    const TRecoilSSDHit& rx2 = recoilSSD->fRecoilSSDHits[recoilIndex[1]];
    const TRecoilSSDHit& ry1 = recoilSSD->fRecoilSSDHits[recoilIndex[2]];
    ClusterSSDX1 = cx1.StripXYPositionWorld;
    ClusterSSDZX1 = cx1.StripZPositionWorld;
    ClusterSSDY1 = cy1.StripXYPositionWorld;
    ClusterSSDZY1 = cy1.StripZPositionWorld;
    ClusterSSDX2 = cx2.StripXYPositionWorld;
    ClusterSSDZX2 = cx2.StripZPositionWorld;
    ClusterSSDY2 = cy2.StripXYPositionWorld;
    ClusterSSDZY2 = cy2.StripZPositionWorld;
    RecoilSSDX1Locus = rx1.StripXYPositionLocus;
    RecoilSSDX2Locus = rx2.StripXYPositionLocus;
    RecoilSSDY1Locus = ry1.StripXYPositionLocus;
    VertexX = reaction->GetVertexPositionX();
    VertexY = reaction->GetVertexPositionY();
    VertexZ = reaction->GetVertexPositionZ();
    Double_t values[14] = {ClusterSSDX1, ClusterSSDZX1, ClusterSSDY1, ClusterSSDZY1, ClusterSSDX2, ClusterSSDZX2, ClusterSSDY2, ClusterSSDZY2, RecoilSSDX1Locus, RecoilSSDX2Locus, RecoilSSDY1Locus, VertexX, VertexY, VertexZ};
    for (Int_t i = 0; i < 14; i++) {
      if (!std::isfinite(values[i]) || values[i] == kInvalidValue)
        valid = false;
    }
    if (!valid)
      continue;
    EventID = static_cast<Long64_t>(firstCluster.EventID);
    if (EventID < 0 || !writtenEventIDs.insert(EventID).second)
      throw std::runtime_error("Invalid or duplicate EventID in selected events; cannot align uniquely.");
    outputTree->Fill();
  }
  std::cout << "Input events: " << entries << std::endl;
  std::cout << "Four-layer same-track alpha events: " << singleCluster << std::endl;
  std::cout << "Also three-layer same-track proton events: " << singleRecoil << std::endl;
  std::cout << "Matched Left-RU/RD or Right-LU/LD events: " << matchedPairs << std::endl;
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
  TString vertexDir = gSystem->DirName(__FILE__);
  TString trainInput = vertexDir + "/../EnergyLoss/alphap.root";
  TString testInput = vertexDir + "/../EnergyLoss/alphap_test.root";
  TString trainOutput = vertexDir + "/Processd.root";
  TString testOutput = vertexDir + "/Processd_test.root";
  ProcessVertexFile(trainInput.Data(), trainOutput.Data());
  ProcessVertexFile(testInput.Data(), testOutput.Data());
}
