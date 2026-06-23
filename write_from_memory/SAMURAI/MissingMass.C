#include "RtypesCore.h"
#include "TObject.h"
#include "TowBodyKinematics.cc"
#include "TowBodyKinematics.h"
#include <TCanvas.h>
#include <TCutG.h>
#include <TDatime.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include <TMultiGraph.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <iostream>

void DrawKinematicsCurves(TMultiGraph *mg, Color_t color = kRed + 1,
                          Width_t width = 1) {
  TList *graphList = mg->GetListOfGraphs();
  for (int i = 0; i < graphList->GetSize(); i++) {
    TGraph *graph = (TGraph *)graphList->At(i);
    graph->SetLineColor(color);
    graph->SetLineWidth(width);
    graph->Draw("L Same");
  }
}

void MissingMass() {
  const TString inputDir =
      "/Users/yemingxin/anaroot/output/TOGAXSI/20260603_0554/";
  const TString rootFilePath = inputDir + "/run1453.root";
  const TString sbtFilePath =
      "/Users/yemingxin/anaroot/output/ridf2SBT/20260603_1705/hSBT1453.root";
  const TString outputDir = inputDir;
  gSystem->mkdir(outputDir.Data(), kTRUE);

  // TOGAXSI的tree
  TFile *f = TFile::Open(rootFilePath, "READ");
  TTree *t = (TTree *)f->Get("tree");

  // sbt的TTree
  TFile *fsbt = TFile::Open(sbtFilePath, "READ");
  TTree *tsbt = (TTree *)fsbt->Get("tree");
  t->AddFriend(tsbt);

  // 加载cut
  gROOT->ProcessLine(".x cut/cut_gs.C");
  TCutG *cutg = (TCutG *)gROOT->FindObject("cut_gs");
  cutg->SetVarX("theta[1]*TMath::RadToDeg()"); // 弧度变角度
  cutg->SetVarY("segene[1]");

  // 开始画图-----------------------------
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  t->Draw("segene[1]:theta[1]*TMath::RadToDeg()>>h1(200,0,90,600,10,310)");

  TH2F *h1 = (TH2F *)gDirectory->Get(
      "h1"); // 从 ROOT 当前目录 gDirectory 里面，取出名字叫 "h1"
             // 的对象，并把它当作二维直方图 TH2F* 使用
  h1->SetDirectory(0);
  h1->SetTitle("segene[1]:theta[1];#theta^{lab};Energy(Mev/u)");

  auto *proton = TowBodyKinematicsEnergyAngleProton(205.0, 0.0, 3.0155, 1.00728,
                                                    1.00728, 3.0155);

  DrawKinematicsCurves(proton);
  gPad->SetTitle();
  gPad->SetGrid();

  TDatime now;
  TString timestamp =
      Form("%04d%02d%02d_%02d%02d", now.GetYear(), now.GetMonth(), now.GetDay(),
           now.GetHour(), now.GetMinute());
  c1->SaveAs(Form("%s/kinematics_%s.pdf", outputDir.Data(), timestamp.Data()));

  // 使用 TTreeReader 读取 TOGAXSI tree 和 friend
  // tree并计算激发能谱-------------------------------------------

  const Double_t amu = 931.49410242;
  const Double_t ma = 3.0155 * amu;
  const Double_t mb = 1.00728 * amu;
  const Double_t mc = 1.00728 * amu;
  const Double_t md = 3.0155 * amu;

  TTreeReader reader(t); // read是自己定义的变量名字，t是TOGAXSI树的名字

  // segene和theta是数组，所以用Array
  TTreeReaderArray<Double_t> segene(reader, "segene");
  TTreeReaderArray<Double_t> theta(reader, "theta");

  // BeamEnergy 来自 friend tree，前面 AddFriend 的 alias 是 "tsbt"
  // BeamEnergy是普通的数子，所以用TTreeReaderValue
  TTreeReaderValue<Double_t> BeamEnergy(reader, "tsbt.BeamEnergy");

  Long64_t nentries = t->GetEntries();
  Long64_t nentriesSBT = tsbt->GetEntries();

  std::cout << "nentries    = " << nentries << std::endl;
  std::cout << "nentriesSBT = " << nentriesSBT << std::endl;

  // 开始循环
  while (reader.Next()) {
    if (segene[1] <= 0 || theta[1] <= 0 || BeamEnergy <= 0)
      continue;

    // 基态cut
    if (!cutg->IsInside(theta[1] * TMath::RadToDeg(), segene[1]))
      continue;

    Double_t Ta = BeamEnergy * 3.0;
    Double_t Tc = segene[1];
    Double_t EaLab = Ta + ma;
    Double_t s = ma * ma + mb * mb + 2.0 * mb * EaLab;
    Double_t pa = TMath::Sqrt(Ta * Ta + 2.0 * ma * Ta);
    Double_t pc = TMath::Sqrt(Tc * Tc + 2.0 * mc * Tc);
    Double_t mass2 = s + mc * mc - 2.0 * (EaLab + mb) * (Tc + mc) +
                     2.0 * pa * pc * TMath::Cos(theta[1]);

    if (mass2 <= 0)
      continue;

    Double_t Ex = TMath::Sqrt(mass2) - md;
    hEx->Fill(Ex);
  }
}
