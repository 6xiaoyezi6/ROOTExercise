#include "RtypesCore.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"

void GenerateFitGaussData() 
{
  TFile*fout= new TFile("FitGaussData.root","recreate");
  TTree*tree=new TTree("tree","simple event tree");
  TDirectory* histDir = fout->mkdir("histograms");

  Int_t evt;
  Float_t Ex;

  tree->Branch("evt",&evt,"evt/I");
  tree->Branch("Ex",&Ex,"Ex/F");

  TCanvas*c = new TCanvas("c","c",800,600);
  TH1F* h1 = new TH1F("h1","Ex;Ex(adc);counts",1000,0,2000);

  TRandom r(0);
  for(evt=0;evt<50000;evt++)
  {
    if(r.Uniform(0,1)<0.65)
      Ex=r.Gaus(950,70);
    else
      Ex=r.Gaus(1350,90);

    tree->Fill(); //在事件循环中填入(把Ex填入tree的branch中)
    h1->Fill(Ex); //把TH1F写入root文件中的TDirectory文件夹中
  }

  fout->cd();
  tree->Write();

  histDir->cd();
  h1->Write();
  
  fout->Close();
}
