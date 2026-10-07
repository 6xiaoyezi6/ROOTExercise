/*
 * 功能：读取预处理 ROOT 的 tree，绘制原始 GAGG ΔE–E 计数图及真实 PID 散点图。
 * 方法：左图使用两轴 0.5 MeV 等宽 bin、COLZ 和对数色标；右图用五种颜色区分粒子。
 * 注意事项：能量单位 MeV；逐行计数，不归一化、不附加筛选；PID 为模拟真值 0–4。
 *           默认读取 PreProcess.root，可传入 PreProcessSSD.root；仅显示画布，不保存文件。
 *           最后一个 bin 包含上边界，保持与 NumPy histogram2d 的计数语义一致。
 */
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TColor.h"
#include "TString.h"
#include <algorithm>
#include <cmath>

void PlotPID(const char* inputPath = "/Users/yemingxin/ROOT_Exercise/MachineLearning/PID/PreProcess.root")
{
  TFile* f = TFile::Open(inputPath, "READ");
  TTree* tree = (TTree*)f->Get("tree");
  Double_t clusterEnergy = 0, deltaEnergy = 0;
  Int_t pid = -1;
  tree->SetBranchAddress("ClusterGAGGEnergy", &clusterEnergy);
  tree->SetBranchAddress("DeltaEGAGGEnergy", &deltaEnergy);
  tree->SetBranchAddress("PID", &pid);
  const Double_t binWidth = 0.5;
  Int_t nx = std::max(1, (Int_t)std::ceil(tree->GetMaximum("ClusterGAGGEnergy") / binWidth));
  Int_t ny = std::max(1, (Int_t)std::ceil(tree->GetMaximum("DeltaEGAGGEnergy") / binWidth));
  Double_t xmax = nx * binWidth, ymax = ny * binWidth;
  TH2D* h1 = new TH2D("h1", "TOGAXSI #DeltaE-E correlation;ClusterGAGGEnergy (MeV);DeltaEGAGGEnergy (MeV)", nx, 0, xmax, ny, 0, ymax);
  h1->SetDirectory(nullptr);
  TGraph* graphs[5];
  const char* names[5] = {"proton", "d", "t", "^{3}He", "^{4}He"};
  const char* colors[5] = {"#0072B2", "#E69F00", "#009E73", "#D55E00", "#CC79A7"};
  for (Int_t i = 0; i < 5; i++) {
    graphs[i] = new TGraph();
    graphs[i]->SetName(Form("g%d", i + 1));
    graphs[i]->SetMarkerStyle(20);
    graphs[i]->SetMarkerSize(0.3);
    graphs[i]->SetMarkerColorAlpha(TColor::GetColor(colors[i]), 0.35);
  }
  for (Long64_t entry = 0; entry < tree->GetEntries(); entry++) {
    tree->GetEntry(entry);
    Double_t x = clusterEnergy, y = deltaEnergy;
    if (x == xmax) x = std::nextafter(xmax, 0.0);
    if (y == ymax) y = std::nextafter(ymax, 0.0);
    h1->Fill(x, y);
    graphs[pid]->SetPoint(graphs[pid]->GetN(), clusterEnergy, deltaEnergy);
  }
  f->Close();
  delete f;
  gStyle->SetOptStat(0); // 与示例一致，不显示统计框。
  gStyle->SetPalette(kViridis);
  TCanvas* c1 = new TCanvas("c1", "TOGAXSI PID", 1600, 700);
  c1->Divide(2, 1);
  c1->cd(1);
  gPad->SetLeftMargin(0.12);
  gPad->SetRightMargin(0.22);
  gPad->SetBottomMargin(0.13);
  gPad->SetLogz();
  h1->GetXaxis()->CenterTitle();
  h1->GetYaxis()->CenterTitle();
  h1->GetZaxis()->SetTitle(Form("Counts / (%.3g MeV #times %.3g MeV) bin", h1->GetXaxis()->GetBinWidth(1), h1->GetYaxis()->GetBinWidth(1)));
  h1->SetMinimum(1);
  h1->SetMaximum(std::max(2.0, h1->GetMaximum()));
  h1->Draw("COLZ");
  c1->cd(2);
  gPad->SetLeftMargin(0.12);
  gPad->SetRightMargin(0.04);
  gPad->SetBottomMargin(0.13);
  TH2D* h2 = new TH2D("h2", "TOGAXSI #DeltaE-E distribution by true PID;ClusterGAGGEnergy (MeV);DeltaEGAGGEnergy (MeV)", 1, 0, xmax, 1, 0, ymax);
  h2->SetDirectory(nullptr);
  h2->GetXaxis()->CenterTitle();
  h2->GetYaxis()->CenterTitle();
  h2->Draw("AXIS");
  TLegend* legend = new TLegend(0.79, 0.72, 0.95, 0.89);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetEntrySeparation(0);
  for (Int_t i = 0; i < 5; i++) {
    graphs[i]->Draw("P SAME");
    legend->AddEntry(graphs[i], names[i], "p");
  }
  legend->Draw();
  c1->Modified();
  c1->Update();
  c1->SaveAs("PID.pdf");
}
