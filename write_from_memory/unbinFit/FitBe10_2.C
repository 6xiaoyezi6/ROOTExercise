/*
  FitBe10_2.C：读取 generate.root 中的 hExBe10，对 10Be 激发能谱进行
  分箱扩展最大似然拟合。

  本版本与 FitBe10.C 的主要区别：
  1. 不再使用 TF1 在负激发能侧带预拟合本底，也不预先固定本底形状和计数。
  2. 第一套拟合使用 RooPolynomial 定义二次本底 PDF：
     1 + backgroundSlope*exEnergy + backgroundQuadratic*exEnergy^2。
  3. 第二套独立拟合使用 RooExponential 定义指数本底 PDF：
     exp(backgroundTau*exEnergy)。
  4. 两套拟合的本底形状参数和 nBackground 都作为自由参数，与所有峰同时拟合。
  5. 总拟合区间为完整的 [-30, 30] MeV，本底参数由整个区间的数据决定。
  6. 原 RooPolynomial 线性本底代码保留为注释，并说明其斜率区间限制原因。
  7. 峰模型保持不变：基态、3.37 MeV、6.17 MeV 三个高斯峰和
     10.15 MeV Breit-Wigner 分量。
  8. nGs、n337、n617、n1015 和 nBackground 是各子分量在完整拟合区间
     内的扩展拟合产额；本底自由参数与峰参数的相关性会进入拟合误差。
  9. 不输出 CSV；两套拟合都在终端打印基态和第一激发态的 mu、sigma、
     计数及误差，并分别输出 FitBe10_2.pdf 和 FitBe10_2_exponential.pdf。
*/

#include <iomanip>
#include <iostream>
#include <memory>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TLine.h"
#include "TMath.h"
#include "TString.h"
#include "TStyle.h"

#include "RooAddPdf.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "RooBreitWigner.h"
#include "RooDataHist.h"
#include "RooExponential.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooGaussian.h"
#include "RooPlot.h"
#include "RooPolynomial.h"
#include "RooRealVar.h"

using namespace RooFit;
namespace {

const double kPlotMin = -30.0;
const double kPlotMax = 30.0;
const double kFitMin = -30.0;
const double kFitMax = 30.0;

void setHistAxisTitle(TH1 *histogram)
{
  const double binWidth = histogram->GetXaxis()->GetBinWidth(1);
  histogram->GetXaxis()->SetTitle("Excitation energy (MeV)");
  histogram->GetYaxis()->SetTitle(Form("Counts / %.3g MeV", binWidth));
}

void drawVerticalLine(double x, double yMin, double yMax)
{
  TLine *line = new TLine(x, yMin, x, yMax);
  line->SetLineColor(kBlack);
  line->SetLineStyle(2);
  line->SetLineWidth(2);
  line->SetBit(kCanDelete);
  line->Draw();
}

}

void FitBe10_2()
{
  gStyle->SetOptFit(0);

  TFile *inputFile = TFile::Open("/Users/yemingxin/ROOT_Exercise/write_from_memory/unbinFit/generate.root", "READ");
  TH1 *hEx = (TH1*)inputFile->Get("hExBe10");

  setHistAxisTitle(hEx);

  RooRealVar exEnergy("exEnergy", "Excitation energy", kPlotMin, kPlotMax, "MeV");
  exEnergy.setRange("fit", kFitMin, kFitMax);
  RooDataHist data("data", "binned hExBe10", RooArgList(exEnergy), Import(*hEx));

  const double nObserved = data.sumEntries(0, "fit");

  RooRealVar muGs("mu_gs", "ground-state position", 0.0, -0.5, 1.0, "MeV");
  RooRealVar sigmaGs("sigma_gs", "ground-state width", 0.7, 0.15, 1.5, "MeV");
  RooGaussian groundState("ground_state", "ground state", exEnergy, muGs, sigmaGs);

  RooRealVar mu337("mu_337", "3.37-MeV-state position", 3.37, 3.1, 3.6, "MeV");
  RooRealVar sigma337("sigma_337", "3.37-MeV-state width", 0.8, 0.2, 2.0, "MeV");
  RooGaussian state337("state_337", "3.37 MeV state", exEnergy, mu337, sigma337);

  RooRealVar mu617("mu_617", "6.17-MeV-state position", 6.17, 5.9, 6.2, "MeV");
  RooRealVar sigma617("sigma_617", "6.17-MeV-state width", 0.5, 0.2, 1.5, "MeV");
  RooGaussian state617("state_617", "6.17 MeV state", exEnergy, mu617, sigma617);

  RooRealVar mu1015("mu_1015", "10.15-MeV-state position", 10.15, "MeV");
  mu1015.setConstant(kTRUE);
  RooRealVar gamma1015("gamma_1015", "10.15-MeV-state width", 0.3, 0.01, 3.0, "MeV");
  RooBreitWigner state1015("state_1015", "10.15 MeV state", exEnergy, mu1015, gamma1015);

  // 原线性本底为 background(x)=1+backgroundSlope*x。PDF 在整个拟合区间内必须非负。
  // 当 x 属于 [-30,30] MeV 时，必须同时满足 1-30*backgroundSlope>=0 和
  // 1+30*backgroundSlope>=0，因此 backgroundSlope 必须位于约 [-0.03333,0.03333] MeV^{-1}。
  // 代码取 [-0.033,0.033] 而不取边界值，是为了避免 PDF 在端点等于零造成数值不稳定。
  // RooRealVar backgroundSlope("background_slope", "linear-background slope", 0.0, -0.033, 0.033, "MeV^{-1}");
  // RooPolynomial linearBackground("linear_background", "linear background", exEnergy, RooArgList(backgroundSlope));

  // 二次本底为 1+backgroundSlope*x+backgroundQuadratic*x^2。
  // backgroundSlope 限制在 [-0.033,0.033]，保证单独的线性部分在全区间为正；
  // backgroundQuadratic 限制为非负，因此二次项只会增加函数值，不会令 PDF 变成负数。
  // 二次项上限从 0.05 按 (20/30)^2 缩放到约 0.022，使端点允许的最大弯曲程度基本不变。
  RooRealVar backgroundSlope("background_slope", "linear-background slope", 0.0, -0.033, 0.033, "MeV^{-1}");
  RooRealVar backgroundQuadratic("background_quadratic", "quadratic-background coefficient", 0.001, 0.0, 0.022, "MeV^{-2}");
  RooPolynomial background("background", "quadratic background", exEnergy, RooArgList(backgroundSlope, backgroundQuadratic));

  const double yieldMaximum = TMath::Max(1000.0, 2.0*nObserved);
  RooRealVar nGs("n_gs", "ground-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  RooRealVar n337("n_337", "3.37-MeV-state yield", TMath::Max(1.0, 0.15*nObserved), 0.0, yieldMaximum);
  RooRealVar n617("n_617", "6.17-MeV-state yield", TMath::Max(1.0, 0.55*nObserved), 0.0, yieldMaximum);
  RooRealVar n1015("n_1015", "10.15-MeV-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  RooRealVar nBackground("n_background", "background yield", TMath::Max(1.0, 0.20*nObserved), 0.0, yieldMaximum);

  RooArgList components;
  components.add(groundState);
  components.add(state337);
  components.add(state617);
  components.add(state1015);
  components.add(background);

  RooArgList yields;
  yields.add(nGs);
  yields.add(n337);
  yields.add(n617);
  yields.add(n1015);
  yields.add(nBackground);

  RooAddPdf model("model", "ground + 3.37 MeV + 6.17 MeV + 10.15 MeV + background", components, yields);

  model.fixCoefRange("fit");
  RooArgSet minosParameters;
  minosParameters.add(nGs);
  minosParameters.add(n337);
  std::unique_ptr<RooFitResult> fitResult(model.fitTo(data, Range("fit"), Extended(kTRUE), Save(kTRUE), Minos(minosParameters), Strategy(2), PrintLevel(-1)));

  double nGsErrorLow = nGs.getAsymErrorLo();
  if(nGsErrorLow < 0.0){
    nGsErrorLow = -nGsErrorLow;
  }
  else{
    nGsErrorLow = nGs.getError();
  }

  double nGsErrorHigh = nGs.getAsymErrorHi();
  if(nGsErrorHigh <= 0.0){
    nGsErrorHigh = nGs.getError();
  }

  double n337ErrorLow = n337.getAsymErrorLo();
  if(n337ErrorLow < 0.0){
    n337ErrorLow = -n337ErrorLow;
  }
  else{
    n337ErrorLow = n337.getError();
  }

  double n337ErrorHigh = n337.getAsymErrorHi();
  if(n337ErrorHigh <= 0.0){
    n337ErrorHigh = n337.getError();
  }

  const double nExpected = nGs.getVal() + n337.getVal() + n617.getVal() + n1015.getVal() + nBackground.getVal();

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "\n========== FitBe10_2 fit results ==========\n";
  std::cout << "Ground state:\n";
  std::cout << "  mu     = " << muGs.getVal() << " +/- " << muGs.getError() << " MeV\n";
  std::cout << "  sigma  = " << sigmaGs.getVal() << " +/- " << sigmaGs.getError() << " MeV\n";
  std::cout << "  counts = " << nGs.getVal() << " - " << nGsErrorLow << " + " << nGsErrorHigh << "\n";
  std::cout << "First excited state (3.37 MeV):\n";
  std::cout << "  mu     = " << mu337.getVal() << " +/- " << mu337.getError() << " MeV\n";
  std::cout << "  sigma  = " << sigma337.getVal() << " +/- " << sigma337.getError() << " MeV\n";
  std::cout << "  counts = " << n337.getVal() << " - " << n337ErrorLow << " + " << n337ErrorHigh << "\n";
  std::cout << "Fit status = " << fitResult->status() << ", covariance quality = " << fitResult->covQual() << "\n";
  std::cout << "Observed counts in fit range = " << nObserved << "\n";
  std::cout << "Expected counts in fit range = " << nExpected << "\n";
  std::cout << "===========================================\n" << std::endl;

  TCanvas *canvas = new TCanvas("canvas_FitBe10_2", "FitBe10_2", 900, 700);
  RooPlot *frame = exEnergy.frame(Title("^{10}Be excitation energy"), Bins(hEx->GetNbinsX()));

  data.plotOn(frame, Name("data"));
  model.plotOn(frame, Range("fit"), NormRange("fit"), Name("total"), LineColor(kRed+1), LineWidth(3));
  model.plotOn(frame, Components(groundState), Range("fit"), NormRange("fit"), Name("ground_curve"), LineColor(kBlue+1), LineWidth(2));
  model.plotOn(frame, Components(state337), Range("fit"), NormRange("fit"), Name("state337_curve"), LineColor(kGreen+2), LineWidth(2));
  model.plotOn(frame, Components(state617), Range("fit"), NormRange("fit"), Name("state617_curve"), LineColor(kOrange+7), LineWidth(2));
  model.plotOn(frame, Components(state1015), Range("fit"), NormRange("fit"), Name("state1015_curve"), LineColor(kCyan+2), LineWidth(2));
  model.plotOn(frame, Components(background), Range("fit"), NormRange("fit"), Name("background_curve"), LineColor(kMagenta+2), LineStyle(2), LineWidth(2));

  const double plotBinWidth = hEx->GetXaxis()->GetBinWidth(1);
  frame->GetXaxis()->SetTitle("Excitation energy (MeV)");
  frame->GetYaxis()->SetTitle(Form("Counts / %.3g MeV", plotBinWidth));
  double yMaximum = frame->GetMaximum()*1.20;
  frame->SetMaximum(yMaximum);

  frame->Draw();

  drawVerticalLine(0.0, 0.0, yMaximum);
  drawVerticalLine(3.37, 0.0, yMaximum);
  drawVerticalLine(6.17, 0.0, yMaximum);
  drawVerticalLine(10.15, 0.0, yMaximum);

  int oldLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  canvas->Print("FitBe10_2.pdf");
  gErrorIgnoreLevel = oldLevel;

  // 第二套拟合：使用一套独立的峰参数和产额参数，将二次本底替换为指数本底。
  RooRealVar muGsExp("mu_gs_exp", "ground-state position", 0.0, -0.5, 1.0, "MeV");
  RooRealVar sigmaGsExp("sigma_gs_exp", "ground-state width", 0.7, 0.15, 1.5, "MeV");
  RooGaussian groundStateExp("ground_state_exp", "ground state", exEnergy, muGsExp, sigmaGsExp);

  RooRealVar mu337Exp("mu_337_exp", "3.37-MeV-state position", 3.37, 3.1, 3.6, "MeV");
  RooRealVar sigma337Exp("sigma_337_exp", "3.37-MeV-state width", 0.8, 0.2, 2.0, "MeV");
  RooGaussian state337Exp("state_337_exp", "3.37 MeV state", exEnergy, mu337Exp, sigma337Exp);

  RooRealVar mu617Exp("mu_617_exp", "6.17-MeV-state position", 6.17, 5.9, 6.2, "MeV");
  RooRealVar sigma617Exp("sigma_617_exp", "6.17-MeV-state width", 0.5, 0.2, 1.5, "MeV");
  RooGaussian state617Exp("state_617_exp", "6.17 MeV state", exEnergy, mu617Exp, sigma617Exp);

  RooRealVar mu1015Exp("mu_1015_exp", "10.15-MeV-state position", 10.15, "MeV");
  mu1015Exp.setConstant(kTRUE);
  RooRealVar gamma1015Exp("gamma_1015_exp", "10.15-MeV-state width", 0.3, 0.01, 3.0, "MeV");
  RooBreitWigner state1015Exp("state_1015_exp", "10.15 MeV state", exEnergy, mu1015Exp, gamma1015Exp);

  RooRealVar backgroundTau("background_tau", "exponential-background slope", 0.0, -1.0, 1.0, "MeV^{-1}");
  RooExponential backgroundExp("background_exp", "exponential background", exEnergy, backgroundTau);

  RooRealVar nGsExp("n_gs_exp", "ground-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  RooRealVar n337Exp("n_337_exp", "3.37-MeV-state yield", TMath::Max(1.0, 0.15*nObserved), 0.0, yieldMaximum);
  RooRealVar n617Exp("n_617_exp", "6.17-MeV-state yield", TMath::Max(1.0, 0.55*nObserved), 0.0, yieldMaximum);
  RooRealVar n1015Exp("n_1015_exp", "10.15-MeV-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  RooRealVar nBackgroundExp("n_background_exp", "background yield", TMath::Max(1.0, 0.20*nObserved), 0.0, yieldMaximum);

  RooArgList componentsExp;
  componentsExp.add(groundStateExp);
  componentsExp.add(state337Exp);
  componentsExp.add(state617Exp);
  componentsExp.add(state1015Exp);
  componentsExp.add(backgroundExp);

  RooArgList yieldsExp;
  yieldsExp.add(nGsExp);
  yieldsExp.add(n337Exp);
  yieldsExp.add(n617Exp);
  yieldsExp.add(n1015Exp);
  yieldsExp.add(nBackgroundExp);

  RooAddPdf modelExp("model_exp", "ground + 3.37 MeV + 6.17 MeV + 10.15 MeV + exponential background", componentsExp, yieldsExp);
  modelExp.fixCoefRange("fit");

  std::unique_ptr<RooFitResult> fitResultExp(modelExp.fitTo(data, Range("fit"), Extended(kTRUE), Save(kTRUE), Strategy(2), PrintLevel(-1)));

  double nGsExpErrorLow = nGsExp.getAsymErrorLo();
  if(nGsExpErrorLow < 0.0){
    nGsExpErrorLow = -nGsExpErrorLow;
  }
  else{
    nGsExpErrorLow = nGsExp.getError();
  }

  double nGsExpErrorHigh = nGsExp.getAsymErrorHi();
  if(nGsExpErrorHigh <= 0.0){
    nGsExpErrorHigh = nGsExp.getError();
  }

  double n337ExpErrorLow = n337Exp.getAsymErrorLo();
  if(n337ExpErrorLow < 0.0){
    n337ExpErrorLow = -n337ExpErrorLow;
  }
  else{
    n337ExpErrorLow = n337Exp.getError();
  }

  double n337ExpErrorHigh = n337Exp.getAsymErrorHi();
  if(n337ExpErrorHigh <= 0.0){
    n337ExpErrorHigh = n337Exp.getError();
  }

  const double nExpectedExp = nGsExp.getVal() + n337Exp.getVal() + n617Exp.getVal() + n1015Exp.getVal() + nBackgroundExp.getVal();

  std::cout << "\n========== FitBe10_2 exponential-background fit results ==========\n";
  std::cout << "Ground state:\n";
  std::cout << "  mu     = " << muGsExp.getVal() << " +/- " << muGsExp.getError() << " MeV\n";
  std::cout << "  sigma  = " << sigmaGsExp.getVal() << " +/- " << sigmaGsExp.getError() << " MeV\n";
  std::cout << "  counts = " << nGsExp.getVal() << " - " << nGsExpErrorLow << " + " << nGsExpErrorHigh << "\n";
  std::cout << "First excited state (3.37 MeV):\n";
  std::cout << "  mu     = " << mu337Exp.getVal() << " +/- " << mu337Exp.getError() << " MeV\n";
  std::cout << "  sigma  = " << sigma337Exp.getVal() << " +/- " << sigma337Exp.getError() << " MeV\n";
  std::cout << "  counts = " << n337Exp.getVal() << " - " << n337ExpErrorLow << " + " << n337ExpErrorHigh << "\n";
  std::cout << "Fit status = " << fitResultExp->status() << ", covariance quality = " << fitResultExp->covQual() << "\n";
  std::cout << "Observed counts in fit range = " << nObserved << "\n";
  std::cout << "Expected counts in fit range = " << nExpectedExp << "\n";
  std::cout << "==================================================================\n" << std::endl;

  TCanvas *canvasExp = new TCanvas("canvas_FitBe10_2_exponential", "FitBe10_2 exponential background", 900, 700);
  RooPlot *frameExp = exEnergy.frame(Title("^{10}Be excitation energy: exponential background"), Bins(hEx->GetNbinsX()));

  data.plotOn(frameExp, Name("data_exp"));
  modelExp.plotOn(frameExp, Range("fit"), NormRange("fit"), Name("total_exp"), LineColor(kRed+1), LineWidth(3));
  modelExp.plotOn(frameExp, Components(groundStateExp), Range("fit"), NormRange("fit"), Name("ground_curve_exp"), LineColor(kBlue+1), LineWidth(2));
  modelExp.plotOn(frameExp, Components(state337Exp), Range("fit"), NormRange("fit"), Name("state337_curve_exp"), LineColor(kGreen+2), LineWidth(2));
  modelExp.plotOn(frameExp, Components(state617Exp), Range("fit"), NormRange("fit"), Name("state617_curve_exp"), LineColor(kOrange+7), LineWidth(2));
  modelExp.plotOn(frameExp, Components(state1015Exp), Range("fit"), NormRange("fit"), Name("state1015_curve_exp"), LineColor(kCyan+2), LineWidth(2));
  modelExp.plotOn(frameExp, Components(backgroundExp), Range("fit"), NormRange("fit"), Name("background_curve_exp"), LineColor(kMagenta+2), LineStyle(2), LineWidth(2));

  frameExp->GetXaxis()->SetTitle("Excitation energy (MeV)");
  frameExp->GetYaxis()->SetTitle(Form("Counts / %.3g MeV", plotBinWidth));
  double yMaximumExp = frameExp->GetMaximum()*1.20;
  frameExp->SetMaximum(yMaximumExp);

  frameExp->Draw();
  drawVerticalLine(0.0, 0.0, yMaximumExp);
  drawVerticalLine(3.37, 0.0, yMaximumExp);
  drawVerticalLine(6.17, 0.0, yMaximumExp);
  drawVerticalLine(10.15, 0.0, yMaximumExp);

  oldLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  canvasExp->Print("FitBe10_2_exponential.pdf");
  gErrorIgnoreLevel = oldLevel;

  inputFile->Close();
}
