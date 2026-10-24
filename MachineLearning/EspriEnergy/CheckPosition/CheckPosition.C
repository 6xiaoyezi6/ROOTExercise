/*
 * 功能：读取 EspriEnergy/InputRootFiles 目录的 Espri.root 和 Ex5sigma.root，按左右臂绘制两行五列能角图，
 *       五列为实验、实验加 BDC-RDC Z 的 5sigma 门、实验同时加 Z 门和 phi12 门、
 *       模拟停止筛选前及筛选后；
 *       另输出原三列 RDCX.pdf、RDCY.pdf，以及两列 RDCX_55_70.pdf、RDCY_55_70.pdf。
 * 方法：实验使用 protonTheta/protonEnergy 及有效所选侧 RDC；模拟沿用 PreProcess.C
 *       的唯一初始质子、同臂塑闪/NaI 正能量、符合臂合计唯一 RDC 命中条件。
 *       将实验约定 RDCX/Y 还原为世界中面位置，减去 InitialConditions 的真实三维
 *       发射顶点，以连线相对世界 +Z 的极角填图，不使用生成动量方向或 RDC 拟合斜率。
 *       第二列仅在第一列基础上加有限 BacktrackingZ_BDCRDC，
 *       |Z-(-4.205798)| <= 5*15.635715 mm，沿用 EfficiencyNew/Draw.C 的参数。
 *       第三列在第二列基础上要求有限 phi12 且 |phi12-180|<15 deg。
 *       近似停止条件为同臂唯一 NaI 质子记录的 Edep >= 首步后动能。
 *       十图均用原始 Counts/bin，共用对数色标，不归一化；沿用 ROOT/anaroot
 *       启动时的默认绘图样式，关闭统计框，能角图横轴标题为 theta_p。
 *       RDC 图直接使用所选事件的实验 rdcX/Y[side] 和模拟 GetRdcPosX/Y，不做坐标变换；
 *       三列依次为实验/模拟、实验加 Z 门/原模拟、实验加 Z 门/近似停止模拟。
 *       新增 RDC 图的实验加同一 Z 5sigma 门，实验和模拟均要求 55<=theta_p<=70 deg；
 *       两列分别对照停止筛选前、近似停止后的模拟，不额外施加 phi12 门。
 *       RDC 范围 0--460 mm，bin 2 mm，红线为实验、蓝线为模拟，保留原始 Counts；
 *       纵轴为对数坐标，下限 0.5，零计数箱不显示，关闭网格。
 * 注意事项：不加 protonTheta>52 或激发能门；phi12 门仅用于能角图第三列，
 *       原 RDC 三列选择保持不变；55--70 deg 门仅用于新增的两张 RDC 图。
 *       能角图角度 45--75 deg，bin 0.2 deg；
 *       能量 0--220 MeV，bin 0.2 MeV。实验角度相对测量束流，模拟相对世界 +Z。
 *       几何取本输入的 Espri.detector/Espri.hh，改变模拟几何后须同步下列参数；
 *       只用 RDCX/Y 时中面法向坐标置零，与保存世界位置可差半个 0.01 mm 中面厚度。
 *       r>1100 mm 识别当前几何的 NaI 交互记录，停止判据不是严格停止真值。
 *       模拟纵轴为初始真实动能，实验为已重建靶点动能；不重新拟合、模拟或标定。
 *       输入和输出均使用下方具名绝对路径，不依赖启动时的工作目录。
 *       五份 PDF 保存于 EspriEnergy/CheckPosition，运行会覆盖该目录内的同名 PDF；
 *       不创建输出目录，不保存 ROOT、PNG、CSV 或文本文件。
 * 运行：在 EspriEnergy 目录执行 root -l -b -q CheckPosition/CheckPosition.C
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TPad.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TVector3.h"
#include "TEspriData.h"
#include "TInitialConditions.h"
#include "TInteractionCoordinates.h"

namespace CheckPositionInternal {
const char* const kSimulationFile = "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/InputRootFiles/Espri.root";
const char* const kExperimentFile = "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/InputRootFiles/Ex5sigma.root";
const char* const kOutputPdf = "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/CheckPosition/CheckPosition.pdf";
const char* const kRdcOutputPdf[2] = {
    "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/CheckPosition/RDCX.pdf",
    "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/CheckPosition/RDCY.pdf"};
const char* const kRdcAngleOutputPdf[2] = {
    "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/CheckPosition/RDCX_55_70.pdf",
    "/Users/yemingxin/ROOT_Exercise/MachineLearning/EspriEnergy/CheckPosition/RDCY_55_70.pdf"};
const char* const kRdcNames[2] = {"RDCX", "RDCY"};
const Double_t kRdcMinMM = 0.0, kRdcMaxMM = 460.0, kRdcBinWidthMM = 2.0;
const Double_t kRdcAngleMinDeg = 55.0, kRdcAngleMaxDeg = 70.0;
const char* const kRdcAngleSamples[3] = {"Experiment BT 5sigma", "Simulation before", "Approximate NaI stop"};
const Double_t kInvalidValue = -999.0;
const Double_t kAngleMin = 45.0, kAngleMax = 75.0, kAngleBinWidth = 0.2;
const Double_t kEnergyMin = 0.0, kEnergyMax = 220.0, kEnergyBinWidth = 0.2;
const char* const kArms[2] = {"Left", "Right"};
const char* const kSamples[4] = {"Experiment", "Experiment: BDC-RDC Z, 5#sigma",
                               "Simulation before", "Approximate NaI stop"};
const Double_t kBacktrackingZMeanMM = -4.205798;
const Double_t kBacktrackingZSigmaMM = 15.635715;
const Double_t kBacktrackingZNSigma = 5.0;
const Double_t kPhi12CenterDeg = 180.0, kPhi12HalfWidthDeg = 15.0;
// 模拟臂 0=+X/Left/实验 side 1，模拟臂 1=-X/Right/实验 side 0。
const Double_t kRdcThetaDeg[2] = {60.333284806151, 60.0};
const Double_t kRdcRadiusMM[2] = {1172.434333449206 - 162.0, 1173.19 - 162.0};
const Double_t kRdcReadoutCenterX[2] = {227.5 - 1.40, 227.5 - 3.35};
const Double_t kRdcReadoutCenterY = 227.5 - 3.0;
const Double_t kNaIMinimumRadiusMM = 1100.0;

Bool_t ValidCoordinate(Double_t value) {
  return std::isfinite(value) && value != kInvalidValue;
}

Bool_t HasCoincidence(TEspriData* espri, Int_t arm) {
  Bool_t plastic = kFALSE, nai = kFALSE;
  for (UInt_t i = 0; i < espri->GetPlasMultEnergy(); ++i) {
    const Double_t energy = espri->GetPlasEnergy(i);
    if (espri->GetPlasEDetectorNbr(i) == arm + 1 && std::isfinite(energy) && energy > 0.0)
      plastic = kTRUE;
  }
  for (Int_t i = 0; i < espri->GetNaIEMult(); ++i) {
    const Double_t energy = espri->GetNaIEEnergy(i);
    if (espri->GetNaIEDetectorNbr(i) == arm && std::isfinite(energy) && energy > 0.0)
      nai = kTRUE;
  }
  return plastic && nai;
}

Bool_t ApproximateNaIStop(TInteractionCoordinates* interactions, Int_t arm) {
  Int_t records = 0;
  Double_t deposit = kInvalidValue, kineticAfterFirstStep = kInvalidValue;
  for (Int_t i = 0; i < interactions->GetDetectedMultiplicity(); ++i) {
    if (interactions->GetParticleName(i) != "proton") continue;
    const Double_t x = interactions->GetDetectedPositionX(i);
    const Double_t y = interactions->GetDetectedPositionY(i);
    const Double_t z = interactions->GetDetectedPositionZ(i);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) continue;
    if (arm == 0 && x <= 0.0) continue;
    if (arm == 1 && x >= 0.0) continue;
    if (x*x + y*y + z*z <= kNaIMinimumRadiusMM*kNaIMinimumRadiusMM) continue;
    ++records;
    deposit = interactions->GetEnergy(i);
    kineticAfterFirstStep = interactions->GetKineticEnergy(i);
  }
  return records == 1 && std::isfinite(deposit) && deposit > 0.0 &&
      std::isfinite(kineticAfterFirstStep) && kineticAfterFirstStep >= 0.0 &&
      deposit >= kineticAfterFirstStep;
}

TVector3 RdcToWorld(Int_t arm, Double_t rdcX, Double_t rdcY) {
  const Double_t theta = kRdcThetaDeg[arm] * TMath::DegToRad();
  const Double_t x = rdcX - kRdcReadoutCenterX[arm];
  const Double_t y = rdcY - kRdcReadoutCenterY;
  if (arm == 0) {
    return TVector3(kRdcRadiusMM[arm]*std::sin(theta) + x*std::cos(theta),
                    y, kRdcRadiusMM[arm]*std::cos(theta) - x*std::sin(theta));
  }
  return TVector3(-kRdcRadiusMM[arm]*std::sin(theta) + x*std::cos(theta),
                  y, kRdcRadiusMM[arm]*std::cos(theta) + x*std::sin(theta));
}

// 只绘制副本，避免第二、三列共用同一实验直方图时互相修改标题或纵轴。
void DrawRDCComparison(TCanvas* c2, TH1D* histograms[2][4], Int_t coordinate,
                       Bool_t angleSelected = kFALSE) {
  c2->Clear();
  Int_t columns = 3;
  if (angleSelected) columns = 2;
  // 在创建子 pad 前固定实际画布大小，避免导出时沿用缩放前的文字坐标。
  const Double_t padGap = 0.01;
  const Double_t frameWidth = (1.0/columns - 2.0*padGap) *
      (1.0 - gStyle->GetPadLeftMargin() - gStyle->GetPadRightMargin());
  const Double_t frameHeight = (0.5 - 2.0*padGap) *
      (1.0 - gStyle->GetPadTopMargin() - gStyle->GetPadBottomMargin());
  const Double_t frameAspect = 4.0 / 3.0;
  c2->SetCanvasSize(1800, TMath::Nint(1800*frameWidth/(frameAspect*frameHeight)));
  gStyle->SetPaperSize(20.0, 20.0*c2->GetWh()/c2->GetWw());
  c2->cd();
  c2->Divide(columns, 2, padGap, padGap);
  const char* experimentLabels[3] = {"Exp", "Exp 5#sigma", "Exp 5#sigma"};
  const char* simulationLabels[3] = {"Sim", "Sim", "Sim stop"};
  for (Int_t arm = 0; arm < 2; ++arm) {
    for (Int_t column = 0; column < columns; ++column) {
      Int_t experimentSample = 0, simulationSample = 2;
      if (column > 0) experimentSample = 1;
      if (column == 2) simulationSample = 3;
      if (angleSelected) {
        experimentSample = 1;
        simulationSample = column + 2;
      }
      TPad* pad = (TPad*)c2->cd(arm*columns + column + 1);
      pad->SetGrid(0, 0);
      pad->SetLogy(1);
      pad->SetLogz(0);
      TH1D* experiment = histograms[arm][experimentSample];
      TH1D* simulation = histograms[arm][simulationSample];
      const Double_t maximum = std::max(experiment->GetBinContent(experiment->GetMaximumBin()),
                                        simulation->GetBinContent(simulation->GetMaximumBin()));
      experiment->SetTitle(Form("%s: #color[632]{%s} / #color[600]{%s}",
          kArms[arm], experimentLabels[column], simulationLabels[column]));
      if (angleSelected) {
        const char* simulationLabel = "Sim";
        if (column == 1) simulationLabel = "Sim stop";
        experiment->SetTitle(Form("%s: #color[632]{Exp BT 5#sigma} / #color[600]{%s}, %.0f#leq#theta_{p}#leq%.0f#circ",
            kArms[arm], simulationLabel, kRdcAngleMinDeg, kRdcAngleMaxDeg));
      }
      experiment->GetXaxis()->SetTitle(Form("%s (mm)", kRdcNames[coordinate]));
      experiment->GetYaxis()->SetTitle(Form("Counts / %.3g mm", experiment->GetBinWidth(1)));
      experiment->GetXaxis()->CenterTitle(kTRUE);
      experiment->GetYaxis()->CenterTitle(kTRUE);
      experiment->SetMinimum(0.5);
      experiment->SetMaximum(std::max(1.0, 2.0*maximum));
      experiment->DrawCopy("HIST");
      simulation->DrawCopy("HIST SAME");
      pad->RedrawAxis();
      pad->Modified();
      pad->Update();
    }
  }
  c2->cd();
  c2->Modified();
  c2->Update();
  if (angleSelected) c2->SaveAs(kRdcAngleOutputPdf[coordinate]);
  else c2->SaveAs(kRdcOutputPdf[coordinate]);
}

} // namespace CheckPositionInternal

void CheckPosition() {
  using namespace CheckPositionInternal;
  const Int_t angleBins = TMath::Nint((kAngleMax - kAngleMin) / kAngleBinWidth);
  const Int_t energyBins = TMath::Nint((kEnergyMax - kEnergyMin) / kEnergyBinWidth);
  TH2D* h1 = new TH2D("h1", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h2 = new TH2D("h2", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h3 = new TH2D("h3", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h4 = new TH2D("h4", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h5 = new TH2D("h5", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h6 = new TH2D("h6", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h7 = new TH2D("h7", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h8 = new TH2D("h8", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* counts[2][4] = {{h1, h2, h3, h4}, {h5, h6, h7, h8}};
  const Int_t rdcBins = TMath::Nint((kRdcMaxMM - kRdcMinMM) / kRdcBinWidthMM);
  TH1D* h9 = new TH1D("h9", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h10 = new TH1D("h10", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h11 = new TH1D("h11", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h12 = new TH1D("h12", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h13 = new TH1D("h13", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h14 = new TH1D("h14", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h15 = new TH1D("h15", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h16 = new TH1D("h16", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h17 = new TH1D("h17", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h18 = new TH1D("h18", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h19 = new TH1D("h19", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h20 = new TH1D("h20", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h21 = new TH1D("h21", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h22 = new TH1D("h22", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h23 = new TH1D("h23", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h24 = new TH1D("h24", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* rdcCounts[2][2][4] = {{{h9, h10, h11, h12}, {h13, h14, h15, h16}},
                                    {{h17, h18, h19, h20}, {h21, h22, h23, h24}}};
  TH2D* h25 = new TH2D("h25", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* h26 = new TH2D("h26", "", angleBins, kAngleMin, kAngleMax, energyBins, kEnergyMin, kEnergyMax);
  TH2D* phiBTCounts[2] = {h25, h26};
  TH2D* energyAnglePanels[2][5] = {{h1, h2, h25, h3, h4}, {h5, h6, h26, h7, h8}};
  TH1D* h27 = new TH1D("h27", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h28 = new TH1D("h28", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h29 = new TH1D("h29", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h30 = new TH1D("h30", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h31 = new TH1D("h31", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h32 = new TH1D("h32", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h33 = new TH1D("h33", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h34 = new TH1D("h34", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h35 = new TH1D("h35", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h36 = new TH1D("h36", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h37 = new TH1D("h37", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  TH1D* h38 = new TH1D("h38", "", rdcBins, kRdcMinMM, kRdcMaxMM);
  // 新图三个样本：实验 BT、模拟筛选前、近似停止模拟；与原 RDC 直方图独立填充。
  TH1D* rdcAngleCounts[2][2][3] = {{{h27, h28, h29}, {h30, h31, h32}},
                                         {{h33, h34, h35}, {h36, h37, h38}}};
  // 复用绘图函数的四样本接口；新增图两列均引用同一实验 BT 直方图。
  TH1D* rdcAnglePanels[2][2][4] = {{{h27, h27, h28, h29}, {h30, h30, h31, h32}},
                                         {{h33, h33, h34, h35}, {h36, h36, h37, h38}}};
  for (Int_t coordinate = 0; coordinate < 2; ++coordinate) {
    for (Int_t arm = 0; arm < 2; ++arm) {
      for (Int_t sample = 0; sample < 3; ++sample) {
        TH1D* h = rdcAngleCounts[coordinate][arm][sample];
        h->SetDirectory(nullptr);
        h->SetStats(kFALSE);
        h->SetFillStyle(0);
        h->SetLineWidth(1);
        if (sample == 0) h->SetLineColor(kRed);
        else h->SetLineColor(kBlue);
      }
    }
  }
  Long64_t phiBTBelow52[2] = {};
  for (Int_t arm = 0; arm < 2; ++arm) {
    phiBTCounts[arm]->SetDirectory(nullptr);
    phiBTCounts[arm]->SetTitle(Form("%s: Experiment: BT 5#sigma + #phi_{12}", kArms[arm]));
  }
  for (Int_t coordinate = 0; coordinate < 2; ++coordinate) {
    for (Int_t arm = 0; arm < 2; ++arm) {
      for (Int_t sample = 0; sample < 4; ++sample) {
        TH1D* h = rdcCounts[coordinate][arm][sample];
        h->SetDirectory(nullptr);
        h->SetStats(kFALSE);
        h->SetFillStyle(0);
        h->SetLineWidth(1);
        if (sample < 2) h->SetLineColor(kRed);
        else h->SetLineColor(kBlue);
      }
    }
  }
  Long64_t below52[2][4] = {};
  Double_t maxWorldResidualMM[2] = {}, maxAngleResidualDeg[2] = {};
  Long64_t auditEntries[2] = {};
  for (Int_t arm = 0; arm < 2; ++arm) {
    for (Int_t sample = 0; sample < 4; ++sample) {
      counts[arm][sample]->SetDirectory(nullptr);
      counts[arm][sample]->SetTitle(Form("%s: %s", kArms[arm], kSamples[sample]));
    }
  }

  TFile* simulationFile = TFile::Open(kSimulationFile, "READ");
  TTree* simulationTree = (TTree*)simulationFile->Get("SimulatedTree");
  TEspriData* espri = nullptr;
  TInitialConditions* initial = nullptr;
  TInteractionCoordinates* interactions = nullptr;
  simulationTree->SetBranchAddress("Espri", &espri);
  simulationTree->SetBranchAddress("InitialConditions", &initial);
  simulationTree->SetBranchAddress("InteractionCoordinates", &interactions);
  for (Long64_t entry = 0; entry < simulationTree->GetEntries(); ++entry) {
    simulationTree->GetEntry(entry);
    Int_t protonIndex = -1, protonCount = 0;
    for (Int_t i = 0; i < initial->GetParticleMultiplicity(); ++i) {
      if (initial->GetParticleName(i) != "proton") continue;
      protonIndex = i;
      ++protonCount;
    }
    if (protonCount != 1) continue;
    const Double_t energy = initial->GetKineticEnergy(protonIndex);
    const TVector3 vertex(initial->GetIncidentPositionX(), initial->GetIncidentPositionY(),
                          initial->GetIncidentPositionZ());
    if (!std::isfinite(energy) || energy <= 0.0 || !ValidCoordinate(vertex.X()) ||
        !ValidCoordinate(vertex.Y()) || !ValidCoordinate(vertex.Z())) continue;
    Int_t selectedArm = -1, selectedHit = -1, validHits = 0;
    for (Int_t arm = 0; arm < 2; ++arm) {
      if (!HasCoincidence(espri, arm)) continue;
      for (UInt_t hit = 0; hit < espri->GetRdcMultPosition(); ++hit) {
        if (espri->GetRdcPosDetectorNbr(hit) != arm) continue;
        if (!ValidCoordinate(espri->GetRdcPosX(hit)) || !ValidCoordinate(espri->GetRdcPosY(hit))) continue;
        selectedArm = arm;
        selectedHit = hit;
        ++validHits;
      }
    }
    if (validHits != 1) continue;
    const TVector3 world = RdcToWorld(selectedArm, espri->GetRdcPosX(selectedHit),
                                      espri->GetRdcPosY(selectedHit));
    const TVector3 direction = world - vertex;
    if (direction.Mag2() <= 0.0) continue;
    const Double_t angle = direction.Theta() * TMath::RadToDeg();
    const Bool_t inRdcAngleRange = angle >= kRdcAngleMinDeg && angle <= kRdcAngleMaxDeg;
    counts[selectedArm][2]->Fill(angle, energy);
    rdcCounts[0][selectedArm][2]->Fill(espri->GetRdcPosX(selectedHit));
    rdcCounts[1][selectedArm][2]->Fill(espri->GetRdcPosY(selectedHit));
    if (inRdcAngleRange) {
      rdcAngleCounts[0][selectedArm][1]->Fill(espri->GetRdcPosX(selectedHit));
      rdcAngleCounts[1][selectedArm][1]->Fill(espri->GetRdcPosY(selectedHit));
    }
    if (angle <= 52.0) ++below52[selectedArm][2];
    if (ApproximateNaIStop(interactions, selectedArm)) {
      counts[selectedArm][3]->Fill(angle, energy);
      rdcCounts[0][selectedArm][3]->Fill(espri->GetRdcPosX(selectedHit));
      rdcCounts[1][selectedArm][3]->Fill(espri->GetRdcPosY(selectedHit));
      if (inRdcAngleRange) {
        rdcAngleCounts[0][selectedArm][2]->Fill(espri->GetRdcPosX(selectedHit));
        rdcAngleCounts[1][selectedArm][2]->Fill(espri->GetRdcPosY(selectedHit));
      }
      if (angle <= 52.0) ++below52[selectedArm][3];
    }
    // 保存世界位置仅用于独立核验，不参与主图角度计算或事件筛选。
    if (espri->GetRdcHitPositionWorldMult() == espri->GetRdcMultPosition()) {
      const TVector3 savedWorld = espri->GetRdcHitPositionWorld(selectedHit);
      maxWorldResidualMM[selectedArm] = std::max(maxWorldResidualMM[selectedArm], (world - savedWorld).Mag());
      maxAngleResidualDeg[selectedArm] = std::max(maxAngleResidualDeg[selectedArm],
          std::abs(angle - (savedWorld - vertex).Theta()*TMath::RadToDeg()));
      ++auditEntries[selectedArm];
    }
  }
  simulationFile->Close();
  delete simulationFile;

  TFile* experimentFile = TFile::Open(kExperimentFile, "READ");
  TTree* experimentTree = (TTree*)experimentFile->Get("tree");
  Int_t side = -1;
  Double_t rdcX[2] = {}, rdcY[2] = {};
  Double_t protonTheta = kInvalidValue, protonEnergy = kInvalidValue;
  Double_t BacktrackingZ_BDCRDC = kInvalidValue;
  Double_t phi12 = kInvalidValue;
  experimentTree->SetBranchAddress("espriSide", &side);
  experimentTree->SetBranchAddress("rdcX", rdcX);
  experimentTree->SetBranchAddress("rdcY", rdcY);
  experimentTree->SetBranchAddress("protonTheta", &protonTheta);
  experimentTree->SetBranchAddress("protonEnergy", &protonEnergy);
  experimentTree->SetBranchAddress("BacktrackingZ_BDCRDC", &BacktrackingZ_BDCRDC);
  experimentTree->SetBranchAddress("phi12", &phi12);
  for (Long64_t entry = 0; entry < experimentTree->GetEntries(); ++entry) {
    experimentTree->GetEntry(entry);
    if (side != 0 && side != 1) continue;
    if (!std::isfinite(rdcX[side]) || !std::isfinite(rdcY[side]) ||
        rdcX[side] <= -900.0 || rdcY[side] <= -900.0) continue;
    if (!std::isfinite(protonTheta) || protonTheta < 0.0 || protonTheta > 180.0 ||
        !std::isfinite(protonEnergy) || protonEnergy <= 0.0) continue;
    const Int_t arm = 1 - side;
    counts[arm][0]->Fill(protonTheta, protonEnergy);
    rdcCounts[0][arm][0]->Fill(rdcX[side]);
    rdcCounts[1][arm][0]->Fill(rdcY[side]);
    if (protonTheta <= 52.0) ++below52[arm][0];
    if (std::isfinite(BacktrackingZ_BDCRDC) &&
        std::abs(BacktrackingZ_BDCRDC - kBacktrackingZMeanMM) <=
            kBacktrackingZNSigma*kBacktrackingZSigmaMM) {
      counts[arm][1]->Fill(protonTheta, protonEnergy);
      rdcCounts[0][arm][1]->Fill(rdcX[side]);
      rdcCounts[1][arm][1]->Fill(rdcY[side]);
      if (protonTheta >= kRdcAngleMinDeg && protonTheta <= kRdcAngleMaxDeg) {
        rdcAngleCounts[0][arm][0]->Fill(rdcX[side]);
        rdcAngleCounts[1][arm][0]->Fill(rdcY[side]);
      }
      if (protonTheta <= 52.0) ++below52[arm][1];
      if (std::isfinite(phi12) && std::abs(phi12-kPhi12CenterDeg) < kPhi12HalfWidthDeg) {
        phiBTCounts[arm]->Fill(protonTheta, protonEnergy);
        if (protonTheta <= 52.0) ++phiBTBelow52[arm];
      }
    }
  }
  experimentFile->Close();
  delete experimentFile;

  const Double_t colorMin = 0.8;
  Double_t colorMax = 0.0;
  for (Int_t arm = 0; arm < 2; ++arm) {
    for (Int_t sample = 0; sample < 4; ++sample) {
      TH2D* h = counts[arm][sample];
      for (Int_t x = 1; x <= angleBins; ++x) {
        for (Int_t y = 1; y <= energyBins; ++y) {
          const Double_t content = h->GetBinContent(x, y);
          colorMax = std::max(colorMax, content);
        }
      }
    }
  }
  if (colorMax <= 0.0) throw std::runtime_error("No entries in the requested display window");
  colorMax *= 1.2;

  for (Int_t arm = 0; arm < 2; ++arm) {
    for (Int_t sample = 0; sample < 4; ++sample) {
      TH2D* h = counts[arm][sample];
      std::cout << kArms[arm] << ": " << kSamples[sample] << ", N=" << h->GetEntries()
                << ", theta<=52: " << below52[arm][sample] << std::endl;
    }
  }
  for (Int_t arm = 0; arm < 2; ++arm) {
    std::cout << kArms[arm] << ": Experiment BT + phi12, N=" << phiBTCounts[arm]->GetEntries()
              << ", theta<=52: " << phiBTBelow52[arm] << std::endl;
  }
  TCanvas* c1 = new TCanvas("c1", "ESPRI energy and position angle", 3000, 1000);
  c1->Divide(5, 2);
  for (Int_t arm = 0; arm < 2; ++arm) {
    for (Int_t column = 0; column < 5; ++column) {
      TPad* pad = (TPad*)c1->cd(arm*5 + column + 1);
      pad->SetRightMargin(0.16); // 为默认 COLZ 色标及其标题留出空间。
      TH2D* h = energyAnglePanels[arm][column];
      h->SetStats(kFALSE);
      h->GetXaxis()->SetTitle("#theta_{p}");
      if (column < 3) h->GetYaxis()->SetTitle("T_{p,reco} at target (MeV)");
      else h->GetYaxis()->SetTitle("T_{p,true} at vertex (MeV)");
      h->GetZaxis()->SetTitle("Counts / bin");
      h->GetXaxis()->CenterTitle(kTRUE);
      h->GetYaxis()->CenterTitle(kTRUE);
      h->GetYaxis()->SetRangeUser(kEnergyMin, kEnergyMax);
      h->SetMinimum(colorMin);
      h->SetMaximum(colorMax);
      h->Draw("COLZ");
    }
  }
  c1->cd();
  c1->Update();
  c1->SaveAs(kOutputPdf);
  for (Int_t coordinate = 0; coordinate < 2; ++coordinate) {
    for (Int_t arm = 0; arm < 2; ++arm) {
      for (Int_t sample = 0; sample < 4; ++sample) {
        TH1D* h = rdcCounts[coordinate][arm][sample];
        std::cout << kRdcNames[coordinate] << " " << kArms[arm] << " " << kSamples[sample]
                  << ": N=" << h->GetEntries() << ", in range=" << h->Integral()
                  << ", underflow=" << h->GetBinContent(0)
                  << ", overflow=" << h->GetBinContent(h->GetNbinsX()+1) << std::endl;
      }
    }
  }
  TCanvas* c2 = new TCanvas("c2", "RDCX: experiment and simulation", 1800, 900);
  DrawRDCComparison(c2, rdcCounts[0], 0);
  DrawRDCComparison(c2, rdcCounts[1], 1);
  for (Int_t coordinate = 0; coordinate < 2; ++coordinate) {
    for (Int_t arm = 0; arm < 2; ++arm) {
      for (Int_t sample = 0; sample < 3; ++sample) {
        TH1D* h = rdcAngleCounts[coordinate][arm][sample];
        std::cout << kRdcNames[coordinate] << " 55-70 deg " << kArms[arm] << " " << kRdcAngleSamples[sample]
                  << ": N=" << h->GetEntries() << ", in range=" << h->Integral()
                  << ", underflow=" << h->GetBinContent(0)
                  << ", overflow=" << h->GetBinContent(h->GetNbinsX()+1) << std::endl;
      }
    }
    DrawRDCComparison(c2, rdcAnglePanels[coordinate], coordinate, kTRUE);
  }
  for (Int_t arm = 0; arm < 2; ++arm) {
    std::cout << kArms[arm] << " world-position audit: " << auditEntries[arm]
              << " entries; max residual=" << maxWorldResidualMM[arm] << " mm" << std::endl;
  }
  std::cout << "Saved: " << kOutputPdf << ", " << kRdcOutputPdf[0] << ", " << kRdcOutputPdf[1]
            << ", " << kRdcAngleOutputPdf[0] << ", " << kRdcAngleOutputPdf[1] << std::endl;
}
