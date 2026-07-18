#include "GraphPlotter.h"

#include <stdexcept>

#include "TAxis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"

void DrawGraphToPdf(const std::string& inputFile,
                    const std::string& graphPath,
                    const std::string& outputPdf) {
  gROOT->SetBatch(true);
  gStyle->SetOptTitle(1);

  TFile file(inputFile.c_str(), "READ");
  TGraph* graph = file.Get<TGraph>(graphPath.c_str());
  

  TCanvas*c1 = new TCanvas("c1", "Energy loss graph", 1000, 750);
  c1->SetGrid();
  c1->SetLogy();

  graph->SetTitle("Proton range in hydrogen;Energy (MeV/u);Range (mg/cm^{2})");
  graph->SetLineColor(kBlue + 1);
  graph->SetLineWidth(3);
  graph->GetXaxis()->SetLimits(
      0.8 * TMath::MinElement(graph->GetN(), graph->GetX()),
      1.2 * TMath::MaxElement(graph->GetN(), graph->GetX()));
  graph->SetMinimum(0.8 * TMath::MinElement(graph->GetN(), graph->GetY()));
  graph->SetMaximum(1.2 * TMath::MaxElement(graph->GetN(), graph->GetY()));
  graph->Draw("AL");

  c1->SaveAs(outputPdf.c_str());
}
