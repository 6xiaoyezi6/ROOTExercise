#if defined(__CLING__)
R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__ADD_INCLUDE_PATH(/opt/homebrew/include)
R__ADD_LIBRARY_PATH(/Users/yemingxin/nptool/NPLib/lib)
R__LOAD_LIBRARY(libNPKinematics)
R__LOAD_LIBRARY(libNPTimer)
R__LOAD_LIBRARY(libNPWorldAndLocus)
R__LOAD_LIBRARY(libNPTrackReconstruction)
R__LOAD_LIBRARY(libNPTOGAXSIEnergy)
#endif

#include "TOGAXSITrack.h"
#include "EnergyReconstruction.h"
#include "EnergyLossDataBase.h"
#include "NPNuclearMass.h"
#include "TTwoBodyKinematics.h"
#include "TWorldAndLocus.h"
#include "MyTools/TTimer.h"
#include "TVirtualPlaneData.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace TOGAXSIReco {

Double_t kInvalidValue = -999.0;
std::vector<TString> groupNames = {"LU", "LD", "RU", "RD", "UP", "DOWN"};

//用于将坐标转为TVector3的自定义函数
TVector3 ReadVector3(YAML::Node node)
{
  std::vector<Double_t> position = node.as<std::vector<Double_t>>();
  return TVector3(position[0], position[1], position[2]);
}

//用于保存 TOGAXSI 探测器某一组的局域坐标系，并提供一个将世界坐标系中的向量转换为局域坐标系的成员函数
struct Frame {
  Bool_t enabled = kFALSE;
  TVector3 origin, x, y, z;//局域坐标系在世界坐标系的原点和三个方向向量
  TVector3 Local(TVector3 v) { return NPL::TWorldAndLocus::WorldToLocusVector(x, y, z, v); }
  //定义一个名为Local的函数，接收一个TVector3类型的参数v，并返回一个TVector3类型的计算结果
};

} // namespace TOGAXSIReco

void ReconstrucionTOGAXSI_Hits(TString inputFileName = "", TString outputDirName = "") {
  using namespace TOGAXSIReco;

  TString InputRootPath = inputFileName;
  if(InputRootPath.IsNull()) InputRootPath = "/Users/yemingxin/nptool/Projects/myTOGAXSI_ppalpha/Reconstruction/InputRootFile/alphap.root";
  TString outdir = outputDirName;
  if(outdir.IsNull()) outdir = "/Users/yemingxin/nptool/Projects/myTOGAXSI_ppalpha/Reconstruction/Scattering/UseClusterTrack/output";
  TString OutputRootPath = outdir + "/ReconstrucionTOGAXSI_Hits.root";
  if(InputRootPath == OutputRootPath) { std::cerr << "输入和输出不能是同一文件" << std::endl; return; }
  gSystem->mkdir(outdir.Data(), kTRUE);
  if(gSystem->CopyFile(InputRootPath.Data(), OutputRootPath.Data(), kTRUE) != 0) {
    std::cerr << "复制模拟文件失败：" << InputRootPath << " -> " << OutputRootPath << std::endl; return;
  }

  TFile* file = TFile::Open(OutputRootPath.Data(), "UPDATE");
  if(!file || file->IsZombie()) { std::cerr << "无法打开输出副本：" << OutputRootPath << std::endl; return; }
  TTree* tree = (TTree*)file->Get("SimulatedTree");
  TClusterSSDData *cs = nullptr;
  TRecoilSSDData *rs = nullptr;
  TClusterGAGGData *cg = nullptr;
  TRecoilGAGGData *rg = nullptr;
  TVirtualPlaneData *vp = nullptr;
  tree->SetBranchAddress("ClusterSSD", &cs);
  tree->SetBranchAddress("RecoilSSD", &rs);
  tree->SetBranchAddress("ClusterGAGG", &cg);
  tree->SetBranchAddress("RecoilGAGG", &rg);
  tree->SetBranchAddress("VirtualPlane", &vp);

  /////////////////// 读取 YAML 配置 ///////////////////
  TString GeometryPath = "/Users/yemingxin/nptool/NPLib/TrackReconstruction/src/TOGAXSI/Test/ssd.yaml";
  NPL::TOGAXSITrack tracker;
  if(!tracker.ReadConfiguration(GeometryPath)) { file->Close(); return; }

  YAML::Node geometry = YAML::LoadFile(GeometryPath.Data());
  Frame frames[6];//创建一个包含 6 个 Frame 结构体对象的数组

  for(Int_t i = 0; i < 6; ++i) {
    YAML::Node g = geometry["recoil"][groupNames[i].Data()];
    frames[i].enabled = g && g["enabled"].as<Bool_t>();
    if(!frames[i].enabled) continue;

    frames[i].origin = ReadVector3(g["origin_world_mm"]);
    frames[i].x = ReadVector3(g["axis_x_world"]);
    frames[i].y = ReadVector3(g["axis_y_world"]);
    frames[i].z = ReadVector3(g["axis_z_world"]);
  }

  /////////////////// 能量重建配置 ///////////////////
  NPL::TOGAXSIEnergyReconstruction energy;
  NPL::TOGAXSIEnergyReconstruction::TargetConfig target;
  target.CenterWorld.SetXYZ(0, 0, 0); target.RadiusMm = 15.5; target.ThicknessMm = 150.0;
  target.AtomicNumber = NPL::EnergyLossDataBase::liquid_hydrogen_z;
  target.AtomicMassAmu = NPL::EnergyLossDataBase::liquid_hydrogen_a;
  target.MeanExcitationEv = NPL::EnergyLossDataBase::liquid_hydrogen_i_ev;
  target.DensityGramPerCm3 = NPL::EnergyLossDataBase::liquid_hydrogen_density;
  if(!energy.ConfigureTarget(target)) { file->Close(); return; }

  Double_t alphaMass = NPL::NuclearMass::Get("4He").MassAMU, protonMass = NPL::NuclearMass::Get("1H").MassAMU;

  /////////////////// 创建输出 TTree ///////////////////
  TTree* outputTree = new TTree("ReconstrcutionTree", "Event-by-event reconstructed quantities");
  Int_t RecoilGroup = -1, ClusterSide = -1;
  Bool_t VertexOK = kFALSE, RecoilTrackOK = kFALSE, BeamTrackOK = kFALSE;
  Bool_t ClusterEnergyOK = kFALSE, RecoilEnergyOK = kFALSE, BeamEnergyOK = kFALSE, ExcitationOK = kFALSE;

  TVector3 ClusterLinePointWorld{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 ClusterLineVectorWorld{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 ClusterLinePointLocus{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 ClusterLineVectorLocus{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 BDCPointWorld{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 BeamDirection{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 RecoilLineVectorLocus{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 RecoilLineVectorWorld{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 ClusterLineVectorBeam{kInvalidValue, kInvalidValue, kInvalidValue};
  TVector3 RecoilLineVectorBeam{kInvalidValue, kInvalidValue, kInvalidValue};

  Double_t VertexLocusX = kInvalidValue, VertexLocusY = kInvalidValue, VertexLocusZ = kInvalidValue;
  Double_t VertexWorldX = kInvalidValue, VertexWorldY = kInvalidValue, VertexWorldZ = kInvalidValue;
  Double_t BeamEnergyPerUAtVertex = kInvalidValue, ClusterReconstructedEnergyPerU = kInvalidValue;
  Double_t RecoilReconstructedEnergyPerU = kInvalidValue, ExcitationEnergy = kInvalidValue;

  outputTree->Branch("RecoilGroup", &RecoilGroup, "RecoilGroup/I");
  outputTree->Branch("ClusterSide", &ClusterSide, "ClusterSide/I");
  outputTree->Branch("VertexOK", &VertexOK, "VertexOK/O");
  outputTree->Branch("RecoilTrackOK", &RecoilTrackOK, "RecoilTrackOK/O");
  outputTree->Branch("BeamTrackOK", &BeamTrackOK, "BeamTrackOK/O");
  outputTree->Branch("ClusterEnergyOK", &ClusterEnergyOK, "ClusterEnergyOK/O");
  outputTree->Branch("RecoilEnergyOK", &RecoilEnergyOK, "RecoilEnergyOK/O");
  outputTree->Branch("BeamEnergyOK", &BeamEnergyOK, "BeamEnergyOK/O");
  outputTree->Branch("ExcitationOK", &ExcitationOK, "ExcitationOK/O");

  outputTree->Branch("ClusterLinePointWorld", &ClusterLinePointWorld);
  outputTree->Branch("ClusterLineVectorWorld", &ClusterLineVectorWorld);
  outputTree->Branch("ClusterLinePointLocus", &ClusterLinePointLocus);
  outputTree->Branch("ClusterLineVectorLocus", &ClusterLineVectorLocus);
  outputTree->Branch("BDCPointWorld", &BDCPointWorld);
  outputTree->Branch("BeamDirection", &BeamDirection);
  outputTree->Branch("RecoilLineVectorLocus", &RecoilLineVectorLocus);
  outputTree->Branch("RecoilLineVectorWorld", &RecoilLineVectorWorld);
  outputTree->Branch("ClusterLineVectorBeam", &ClusterLineVectorBeam);
  outputTree->Branch("RecoilLineVectorBeam", &RecoilLineVectorBeam);

  outputTree->Branch("VertexLocusX", &VertexLocusX, "VertexLocusX/D");
  outputTree->Branch("VertexLocusY", &VertexLocusY, "VertexLocusY/D");
  outputTree->Branch("VertexLocusZ", &VertexLocusZ, "VertexLocusZ/D");
  outputTree->Branch("VertexWorldX", &VertexWorldX, "VertexWorldX/D");
  outputTree->Branch("VertexWorldY", &VertexWorldY, "VertexWorldY/D");
  outputTree->Branch("VertexWorldZ", &VertexWorldZ, "VertexWorldZ/D");
  outputTree->Branch("BeamEnergyPerUAtVertex", &BeamEnergyPerUAtVertex, "BeamEnergyPerUAtVertex/D");
  outputTree->Branch("ClusterReconstructedEnergyPerU", &ClusterReconstructedEnergyPerU, "ClusterReconstructedEnergyPerU/D");
  outputTree->Branch("RecoilReconstructedEnergyPerU", &RecoilReconstructedEnergyPerU, "RecoilReconstructedEnergyPerU/D");
  outputTree->Branch("ExcitationEnergy", &ExcitationEnergy, "ExcitationEnergy/D");

  /////////////////// 逐事例重建 ///////////////////
  Long64_t entries = tree->GetEntries();
  NPL::TTimer timer; timer.StartProgress(entries);

  for(Long64_t entry = 0; entry < entries; ++entry) {
    tree->GetEntry(entry);

    RecoilGroup = ClusterSide = -1;
    VertexOK = RecoilTrackOK = BeamTrackOK = kFALSE;
    ClusterEnergyOK = RecoilEnergyOK = BeamEnergyOK = ExcitationOK = kFALSE;

    //初始化向量
    for (TVector3* vector : {&ClusterLinePointWorld, &ClusterLineVectorWorld,
                         &ClusterLinePointLocus, &ClusterLineVectorLocus,
                         &BDCPointWorld, &BeamDirection, &RecoilLineVectorLocus,
                         &RecoilLineVectorWorld, &ClusterLineVectorBeam, &RecoilLineVectorBeam}) 
    {
      vector->SetXYZ(kInvalidValue, kInvalidValue, kInvalidValue);
    }

    VertexLocusX = VertexLocusY = VertexLocusZ = kInvalidValue;
    VertexWorldX = VertexWorldY = VertexWorldZ = kInvalidValue;
    BeamEnergyPerUAtVertex = ClusterReconstructedEnergyPerU = kInvalidValue;
    RecoilReconstructedEnergyPerU = ExcitationEnergy = kInvalidValue;

    auto&& clusterHits = cs->GetClusterSSDHits();
    auto&& recoilHits = rs->GetRecoilSSDHits();

    std::size_t bdcCount = std::min<std::size_t>(vp->GetBDCHitPositionWorldMult(), vp->fVirtualBDCHitDirectionWorld.size());
    for(std::size_t i = 0; i < bdcCount; ++i) if(vp->GetBDCHitParticleName(i) == "alpha") {
      BDCPointWorld = vp->GetBDCHitPositionWorld(i); BeamDirection = vp->GetBDCHitDirectionWorld(i); break;
    }

    BeamTrackOK = std::isfinite(BDCPointWorld.X()) && std::isfinite(BDCPointWorld.Y()) && std::isfinite(BDCPointWorld.Z()) &&
                    std::isfinite(BeamDirection.X()) && std::isfinite(BeamDirection.Y()) && std::isfinite(BeamDirection.Z()) && BeamDirection.Z() > 0.0;

    for(Int_t g = 0; g < 6 && RecoilGroup < 0; ++g) {
      Bool_t x1 = kFALSE, x2 = kFALSE;
      for(auto&& h : recoilHits) if(h.RecoilSSDGroup == groupNames[g].Data()) {
        if(h.RecoilSSDType == "X1") x1 = kTRUE;
        if(h.RecoilSSDType == "X2") x2 = kTRUE;
      }
      if(x1 && x2) RecoilGroup = g;
    }

    do {
      if(RecoilGroup < 0 || !frames[RecoilGroup].enabled) break;

      TString group = groupNames[RecoilGroup];
      Frame& frame = frames[RecoilGroup];

      auto projection = tracker.RecoilTrack(recoilHits, group);
      if(!projection.ProjectionSuccess) break;

      auto right = tracker.ClusterTrack(clusterHits, "Right"), left = tracker.ClusterTrack(clusterHits, "Left");
      if(right.Success == left.Success) break;

      auto cluster = left; ClusterSide = 1;
      if(right.Success) { cluster = right; ClusterSide = 0; }

      ClusterLineVectorWorld = cluster.DirectionWorld * (1.0 / cluster.DirectionWorld.Z());
      ClusterLinePointWorld = cluster.PointWorld - cluster.PointWorld.Z() * ClusterLineVectorWorld;
      ClusterLineVectorLocus = frame.Local(ClusterLineVectorWorld);
      ClusterLinePointLocus = frame.Local(ClusterLinePointWorld - frame.origin);

      if(BeamTrackOK) ClusterLineVectorBeam = NPL::TWorldAndLocus::WorldToBeamVector(BeamDirection, ClusterLineVectorWorld);

      auto vertex = tracker.Vertex_ByClusterRecoil(cluster, projection);
      if(!vertex.Success) break;

      VertexOK = kTRUE;
      TVector3 v = vertex.PositionWorld;

      VertexWorldX = v.X(); VertexWorldY = v.Y(); VertexWorldZ = v.Z();
      VertexLocusX = vertex.PositionLocal.X(); VertexLocusY = vertex.PositionLocal.Y(); VertexLocusZ = vertex.PositionLocal.Z();

      auto beam = energy.BeamAtVertex(800.0, 2, 4, BDCPointWorld, BeamDirection, v);
      BeamEnergyOK = beam.Success;
      if(beam.Success) BeamEnergyPerUAtVertex = beam.VertexEnergyPerU;

      auto alpha = energy.ClusterAtVertex(clusterHits, cg->GetClusterGAGGHits(), cluster.Group, v, ClusterLineVectorWorld);
      ClusterEnergyOK = alpha.Success;
      if(alpha.Success) ClusterReconstructedEnergyPerU = alpha.VertexEnergyPerU;

      auto recoil = tracker.RecoilTrack(recoilHits, group, &v);
      if(!recoil.DirectionSuccess) break;

      RecoilTrackOK = kTRUE;
      RecoilLineVectorLocus = frame.Local(recoil.DirectionWorld);
      Double_t scale = 1.0 / RecoilLineVectorLocus.Z();
      RecoilLineVectorLocus *= scale;
      RecoilLineVectorWorld = recoil.DirectionWorld * scale;

      if(BeamTrackOK) RecoilLineVectorBeam = NPL::TWorldAndLocus::WorldToBeamVector(BeamDirection, RecoilLineVectorWorld);

      auto proton = energy.RecoilAtVertex(recoilHits, rg->GetRecoilGAGGHits(), group, v, RecoilLineVectorWorld);
      RecoilEnergyOK = proton.Success;
      if(proton.Success) RecoilReconstructedEnergyPerU = proton.VertexEnergyPerU;

      if(!proton.Success || !beam.Success || !BeamTrackOK) break;

      Double_t ex = NPL::TTwoBodyKinematics::MissingMass(alphaMass, protonMass, protonMass, alphaMass,
          beam.VertexEnergyMeV, proton.VertexEnergyMeV, std::cos(RecoilLineVectorWorld.Angle(BeamDirection)));

      if(std::isfinite(ex)) { ExcitationEnergy = ex; ExcitationOK = kTRUE; }

    } while(kFALSE);

    outputTree->Fill();
    timer.UpdateProgress(entry + 1);
  }

  timer.Stop();
  file->cd();
  outputTree->Write("", TObject::kOverwrite);
  file->Close();
}
