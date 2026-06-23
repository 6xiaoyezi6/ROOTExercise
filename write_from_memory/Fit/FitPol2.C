#include "RtypesCore.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TH1.h"
#include "TMath.h"
#include "TRandom.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVector3.h"

Double_t Polynominal2(Double_t *x, Double_t *par) {
  return par[0] + par[1] * x[0] + par[2] * TMath::Power(x[0], 2);
}

void FitPol2() {
  TCanvas *c = new TCanvas("c", "c", 1000, 800);
  TRandom r(1234);
  TVector3 v;
  TGraph *g2 = new TGraph();
  TF1 *fquadra = new TF1("fquadra", Polynominal2, 0, 5, 3);
  fquadra->SetParameters(1, 2, 3);
  for (int i = 0; i < 5; i++) {
    double x = i;
    double y = fquadra->Eval(i) + r.Gaus(0, 1);
    double z = fquadra->Eval(i);

    v.SetXYZ(x, y, z);
    g2->SetPoint(i, v.X(), v.Y());

    std::cout << "y=" << y << std::endl;
    std::cout << "z=" << z << std::endl;
  }
  g2->SetMarkerStyle(21);
  g2->Draw("AP");

  g2->Fit(fquadra);
}
