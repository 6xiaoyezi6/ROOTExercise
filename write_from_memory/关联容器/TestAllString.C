#include <iostream>
#include <string>

#include "RtypesCore.h"
#include "TClusterKnockoutConditions.h"
#include "TFile.h"
#include "TMath.h"
#include "TTrackInfo.h"
#include "TTree.h"

// 演示如何读取并使用 String格式的变量
void TestAllString() {

  TFile* file = TFile::Open("/Users/yemingxin/nptool/Outputs/Simulation/ppalpha.root", "READ");
  TTree* tree = (TTree*)file->Get("SimulatedTree");

  TClusterKnockoutConditions* ckoc = nullptr;
  TTrackInfo* trackInfo = nullptr;

  tree->SetBranchAddress("ClusterKnockoutConditions", &ckoc);
  tree->SetBranchAddress("TrackInfo", &trackInfo);

  std::string modelName;
  std::string beamParticleName;
  std::string particleName;
  std::string particleRole;
  std::string trackParticleName;
  Double_t BeamTheta;
  Double_t BeamPhi;
  Double_t BeamEnergy;
  Int_t BeamA;
  Double_t Theta;
  Double_t Phi;
  Double_t Energy;

  Long64_t nentries = tree->GetEntriesFast();
  for (Long64_t i = 0; i < 1; i++) {
    tree->GetEntry(i);

    std::cout << "================ Entry " << i << " ================" << std::endl;

    // TClusterKnockoutConditions: getters returning const std::string&
    modelName = ckoc->GetModelName();
    std::cout << "ckoc->GetModelName() = " << modelName << std::endl;

    beamParticleName = ckoc->GetBeamParticleName();
    std::cout << "ckoc->GetBeamParticleName() = " << beamParticleName << std::endl;
    
    BeamTheta = ckoc->GetBeamAngleThetaAtVertex();
    BeamPhi = ckoc->GetBeamAnglePhiAtVertex();
    BeamA = ckoc->GetBeamParticleA();
    BeamEnergy = ckoc->GetBeamEnergyAtVertex() * BeamA;
    std::cout << "ckoc->GetBeamAngleThetaAtVertex() = " << BeamTheta * TMath::RadToDeg() << std::endl;
    std::cout << "ckoc->GetBeamAnglePhiAtVertex() = " << BeamPhi * TMath::RadToDeg() << std::endl;
    std::cout << "ckoc->GetBeamEnergyAtVertex() = " << BeamEnergy << " Mev" << std::endl;
    std::cout << std::endl; // 空一行

    // 内层第一个小循环
    UInt_t particleMultiplicity = ckoc->GetParticleMultiplicity();
    std::cout << "ckoc particle multiplicity = " << particleMultiplicity << std::endl;
    for (UInt_t j = 0; j < particleMultiplicity; j++) {

      particleName = ckoc->GetParticleName(j);
      particleRole = ckoc->GetParticleRole(j);
      Theta = ckoc->GetParticleTheta(j);
      Phi = ckoc->GetParticlePhi(j);
      Energy = ckoc->GetParticleKineticEnergy(j);

      std::cout << "ckoc particle " << j << std::endl;
      std::cout << "ckoc->GetParticleName(j) = " << particleName << std::endl;
      std::cout << "ckoc->GetParticleRole(j) = " << particleRole << std::endl;
      std::cout << "ckoc->GetParticleTheta(j) = " << Theta * TMath::RadToDeg() << std::endl;
      std::cout << "ckoc->GetParticlePhi(j) = " << Phi * TMath::RadToDeg() << std::endl;
      std::cout << "ckoc-->GetParticleKineticEnergy(j) = " << Energy <<" Mev" <<std::endl;
      std::cout << std::endl; // 空一行
    }

    // 内层第二个小循环
    //  TTrackInfo: getter returning std::string
    int trackMultiplicity = trackInfo->GetParticleMultiplicity();
    std::cout << "TrackInfo particle multiplicity = " << trackMultiplicity << std::endl;
    for (int j = 0; j < trackMultiplicity; j++) {
      trackParticleName = trackInfo->GetParticleName(j);
      std::cout << "trackInfo->GetParticleName(j) = " << trackParticleName << std::endl;
    }
  }

  file->Close();
}
