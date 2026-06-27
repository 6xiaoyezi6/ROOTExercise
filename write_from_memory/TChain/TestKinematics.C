#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "RtypesCore.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TReactionConditions.h"
#include "TStyle.h"
#include "TTrackInfo.h"
#include "TTree.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.h"


#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"

void TestKinematics() {
  Timer timer; // 记时器
  constexpr Double_t beamEnergyPerU = 150.0;
  const Double_t mass10BeAmu = 10.01134;
  const Double_t massProtonAmu = 1.00728;
  const Double_t Ex1_10Be = 3.37;

  TChain chain("SimulatedTree");
  chain.Add("/Volumes/Lexar/Espri_Scatter/Espri_ex1.root");
  chain.Add("/Volumes/Lexar/Espri_Scatter/Espri_gs1.root");

  TReactionConditions* rc = nullptr;
  chain.SetBranchAddress("ReactionConditions", &rc);

  Double_t ProtonTheta;
  Double_t ProtonEnergy;

  TCanvas* c1 = new TCanvas("c1", "c1", 1000, 800);
  TH2F* h1 = new TH2F("h1", "Theta Energy Correaltion", 400, 50, 90, 2000, 0, 200);
 
  Long64_t nentries = chain.GetEntries();
  for (Long64_t i = 0; i < nentries; i++) {
    chain.GetEntry(i);

    ProtonTheta = rc->GetTheta(0);
    ProtonEnergy = rc->GetKineticEnergy(0);
    h1->Fill(ProtonTheta, ProtonEnergy);

    timer.UpdateProgress(i); // 计时器
  }
  timer.UpdateProgress(nentries); // 计时器
  timer.Stop();                   // 计时器

  c1->cd();
  h1->Draw("COLZ");

  //叠加运动学曲线
  TMultiGraph* protonGs = TwoBodyKinematicsEnergyAngleProton(
      beamEnergyPerU, 0.0,
      mass10BeAmu, massProtonAmu, massProtonAmu, mass10BeAmu,
      kRed+1, 5, kSolid);
  TMultiGraph* protonEx1 = TwoBodyKinematicsEnergyAngleProton(
      beamEnergyPerU, Ex1_10Be,
      mass10BeAmu, massProtonAmu, massProtonAmu, mass10BeAmu,
      kBlue+1, 5, kDashed);

  protonGs->Draw("L");
  protonEx1->Draw("L");


}
