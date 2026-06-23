#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1F.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVector3.h"

void FunctionOfGauss() {
  TFile *f = TFile::Open("FunctionOfGauss.root", "RECREATE");
  TDirectory *histDir = f->mkdir("histograms");
  TTree *tree = new TTree("tree", "GaussTree", 99, gDirectory);

  Double_t tof;
  Double_t beta;
  Double_t gamma;
  Double_t kinematics;

  constexpr Double_t L = 36014.1;             // mm
  constexpr Double_t c = 299.792458;          // mm/ns
  constexpr Double_t m = 3.0155 * 931.494095; // 3H

  tree->Branch("tof", &tof, "tof/D");
  tree->Branch("beta", &beta, "beta/D");
  tree->Branch("gamma", &gamma, "gamma/D");
  tree->Branch("kinematics",&kinematics,"kinematics/D");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  TH1F *h1 = new TH1F("h1", "tof;tof;counts", 100, 206, 214);
  h1->SetDirectory(0);

  TCanvas *c2 = new TCanvas("c2", "c2", 1000, 800);
  TH1F *h2 = new TH1F("h2", "beta;beta;counts", 100, 0.555, 0.58);
  h2->SetDirectory(0);

  TCanvas *c3 = new TCanvas("c3", "c3", 1000, 800);
  TH1F *h3 = new TH1F("h3", "gamma;gamma;counts", 100, 1.21, 1.23);
  h3->SetDirectory(0);

  TCanvas *c4 = new TCanvas("c4", "c4", 1000, 800);
  TH1F *h4 = new TH1F("h4", "kinematics;kinematics(Mev/u);counts", 100, 200, 210);
  h4->SetDirectory(0);

  TRandom r(0);

  for (Int_t i = 0; i < 100000; i++) {
    tof = r.Gaus(210,0.2763);
    beta = L / (tof * c);
    gamma = 1 / TMath::Power(1 - beta * beta, 0.5);
    kinematics = ((gamma - 1) * m)/3; //Mev/u

    tree->Fill();
    h1->Fill(tof);
    h2->Fill(beta);
    h3->Fill(gamma);
    h4->Fill(kinematics);
  }

  //画在交互界面的
  c1->cd();
  h1->Draw();

  c2->cd();
  h2->Draw();

  c3->cd();
  h3->Draw();

  c4->cd();
  h4->Draw();

  //写入ROOT文件的
  tree->Write();

  histDir->cd();
  h1->Write();
  h2->Write();
  h3->Write();
  h4->Write();

  f->Close();
}
