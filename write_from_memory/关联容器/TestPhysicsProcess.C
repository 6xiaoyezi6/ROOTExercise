#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.h"
#include "RtypesCore.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TError.h"
#include "TEspriData.h"
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

void TestPhysicsProcess() {
  Timer timer; // 记时器
  TFile* f = TFile::Open("/Users/yemingxin/nptool/Outputs/Simulation/EspriPhysics2.root", "READ");
  TTree* tree = (TTree*)f->Get("SimulatedTree");

  TReactionConditions* rc = nullptr;
  TEspriData* espri = nullptr;
  TTrackInfo* ti = nullptr;


  tree->SetBranchAddress("Espri", &espri);
  tree->SetBranchAddress("ReactionConditions", &rc);
  tree->SetBranchAddress("TrackInfo", &ti);

  Long64_t nentries = tree->GetEntriesFast();
  //定义一个名为particleCounts的变量，他的数据类型不是数组，而是关联容器 std::map<KeyType, ValueType> mapName;
  //建立了一种对应关系  粒子名字 -> 计数
  std::map<std::string, Long64_t> particleCounts;

  for (Long64_t i = 0; i < nentries; i++) 
  {
    tree->GetEntry(i);

    const int multiplicity = ti->GetParticleMultiplicity();
    for (int j = 0; j < multiplicity; j++) 
    {
      const std::string particleName = ti->GetParticleName(j);
      particleCounts[particleName]++;
    }

    timer.UpdateProgress(i); // 计时器
  }

  timer.UpdateProgress(nentries); // 计时器
  timer.Stop();                   // 计时器


  /*
  for (元素变量 : 容器)
  依次取出 particleCounts 中的每一个元素，
  把这个元素临时叫作 item(自己起的变量名字)
  然后输出 item.first 和 item.second
  因为std::map中的每个元素是一个std::pair,所以有两个成员 item.first和 item.second
  */
 for (const auto& item : particleCounts) 
 {
  std::cout << item.first << " : " << item.second << std::endl;
 }

  f->Close();
}
