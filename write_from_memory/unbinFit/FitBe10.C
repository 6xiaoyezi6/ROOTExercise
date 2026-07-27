/*
  FitBe10.C：读取 generate.root 中的 hExBe10，并对 10Be 激发能谱
  进行分箱扩展最大似然拟合。

  拟合模型、参数初值和拟合区间保持不变：基态、3.37 MeV、6.17 MeV
  三个高斯峰、10.15 MeV Breit-Wigner 分量和固定线性本底。
*/

#include <iomanip>
#include <iostream>
#include <memory>

#include "TCanvas.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1.h"
#include "TLine.h"
#include "TMath.h"
#include "TString.h"
#include "TStyle.h"

#include "RooAbsReal.h"
#include "RooAddPdf.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "RooBreitWigner.h"
#include "RooDataHist.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooGaussian.h"
#include "RooPlot.h"
#include "RooPolynomial.h"
#include "RooRealVar.h"

using namespace RooFit;
namespace {

const double kPlotMin = -20.0;
const double kPlotMax = 20.0;
const double kFitMin = -5.0;
const double kFitMax = 20.0;
const double kBgFitMin = -20.0;
const double kBgFitMax = -1.5;

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

void FitBe10()
{
  gStyle->SetOptFit(0);

  TFile *inputFile = TFile::Open("/Users/yemingxin/ROOT_Exercise/write_from_memory/unbinFit/generate.root","READ");
  TH1 *hEx = (TH1*)inputFile->Get("hExBe10");

  setHistAxisTitle(hEx);

  RooRealVar exEnergy("exEnergy", "Excitation energy",kPlotMin, kPlotMax, "MeV");
  exEnergy.setRange("fit", kFitMin, kFitMax);
  exEnergy.setRange("background", kBgFitMin, kBgFitMax);
  RooDataHist data("data", "binned hExBe10", RooArgList(exEnergy),Import(*hEx));

  const double nObserved = data.sumEntries(0, "fit");
 
  RooRealVar muGs("mu_gs", "ground-state position",0, -0.5, 1.0, "MeV");
  RooRealVar sigmaGs("sigma_gs", "ground-state width",0.7, 0.15, 1.5, "MeV");
  RooGaussian groundState("ground_state", "ground state", exEnergy, muGs, sigmaGs);

  RooRealVar mu337("mu_337", "3.37-MeV-state position", 3.37, 3.1, 3.6, "MeV");
  RooRealVar sigma337("sigma_337", "3.37-MeV-state width", 0.8, 0.2, 2.0, "MeV");
  RooGaussian state337("state_337", "3.37 MeV state", exEnergy, mu337, sigma337);

  RooRealVar mu617("mu_617", "6.17-MeV-state position", 6.17, 5.9, 6.2, "MeV");
  RooRealVar sigma617("sigma_617", "6.17-MeV-state width", 0.5, 0.2, 1.5, "MeV");
  RooGaussian state617("state_617", "6.17 MeV state", exEnergy, mu617, sigma617);

  RooRealVar mu1015("mu_1015", "10.15-MeV-state position", 10.15, "MeV");
  mu1015.setConstant(kTRUE);
  RooRealVar gamma1015("gamma_1015", "10.15-MeV-state width", 0.3, 0.01, 5, "MeV");
  RooBreitWigner state1015("state_1015", "10.15 MeV state", exEnergy, mu1015, gamma1015);

  //RooPolynomial：定义线性本底 PDF 的形状。
  //RooAddPdf：把多个已经定义好的 PDF 分量相加，组成总模型
  //侧带 TH1 预拟合，获得 p0、p1；p1/p0 决定 RooPolynomial 的形状；TF1 积分决定 nBackground；形状和计数均固定后，加入 RooAddPdf 总拟合
  TF1 backgroundFit("background_fit", "pol1", kBgFitMin, kBgFitMax);
  int oldBackgroundLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kError;
  hEx->Fit(&backgroundFit, "QR0N", "", kBgFitMin, kBgFitMax);
  gErrorIgnoreLevel = oldBackgroundLevel;

  const double backgroundP0Value = backgroundFit.GetParameter(0);
  const double backgroundP1Value = backgroundFit.GetParameter(1);
  RooRealVar backgroundSlope("background_slope", "linear-background slope", backgroundP1Value/backgroundP0Value);
  backgroundSlope.setConstant(kTRUE);
  RooPolynomial background("background", "linear background", exEnergy, RooArgList(backgroundSlope));

  const double yieldMaximum = TMath::Max(1000.0, 2.0*nObserved);
  RooRealVar nGs("n_gs", "ground-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  RooRealVar n337("n_337", "3.37-MeV-state yield", TMath::Max(1.0, 0.15*nObserved), 0.0, yieldMaximum);
  RooRealVar n617("n_617", "6.17-MeV-state yield", TMath::Max(1.0, 0.55*nObserved), 0.0, yieldMaximum);
  RooRealVar n1015("n_1015", "10.15-MeV-state yield", TMath::Max(1.0, 0.05*nObserved), 0.0, yieldMaximum);
  const double binWidth = hEx->GetXaxis()->GetBinWidth(1);
  const double nBackgroundValue = backgroundFit.Integral(kFitMin, kFitMax)/binWidth;
  RooRealVar nBackground("n_background", "background yield", nBackgroundValue);
  nBackground.setConstant(kTRUE);

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

  // The five extended coefficients are defined as expected counts in [kFitMin,kFitMax].
  model.fixCoefRange("fit");
  RooArgSet minosParameters;
  minosParameters.add(nGs);
  minosParameters.add(n337);
  minosParameters.add(n617);
  minosParameters.add(n1015);
  std::unique_ptr<RooFitResult> fitResult(model.fitTo(data, Range("fit"), Extended(kTRUE), Save(kTRUE), Minos(minosParameters), Strategy(1), PrintLevel(-1)));

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
  std::cout << "\n========== FitBe10 fit results ==========\n";
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
  std::cout << "=========================================\n" << std::endl;

  TCanvas *canvas = new TCanvas("canvas_FitBe10", "FitBe10", 900, 700);
  RooPlot *frame = exEnergy.frame(Title("^{10}Be excitation energy"), Bins(hEx->GetNbinsX()));

  data.plotOn(frame, Name("data"));
  model.plotOn(frame, Range("fit"), NormRange("fit"),  Name("total"), LineColor(kRed+1), LineWidth(3));
  model.plotOn(frame, Components(groundState), Range("fit"), NormRange("fit"),  Name("ground_curve"), LineColor(kBlue+1), LineWidth(2));
  model.plotOn(frame, Components(state337), Range("fit"), NormRange("fit"),  Name("state337_curve"), LineColor(kGreen+2), LineWidth(2));
  model.plotOn(frame, Components(state617), Range("fit"), NormRange("fit"),  Name("state617_curve"), LineColor(kOrange+7), LineWidth(2));
  model.plotOn(frame, Components(state1015), Range("fit"), NormRange("fit"),  Name("state1015_curve"), LineColor(kCyan+2), LineWidth(2));
  model.plotOn(frame, Components(background), Range("fit"), NormRange("fit"),  Name("background_curve"), LineColor(kMagenta+2), LineStyle(2), LineWidth(2));

  const double plotBinWidth = hEx->GetXaxis()->GetBinWidth(1);
  frame->GetXaxis()->SetTitle("Excitation energy (MeV)");
  frame->GetYaxis()->SetTitle(Form("Counts / %.3g MeV", plotBinWidth));
  double yMaximum = frame->GetMaximum()*1.20;
  frame->SetMaximum(yMaximum);

  frame->Draw();

  drawVerticalLine(0, 0.0, yMaximum);
  drawVerticalLine(3.37, 0.0, yMaximum);
  drawVerticalLine(6.17, 0.0, yMaximum);
  drawVerticalLine(10.15, 0.0, yMaximum);

  int oldLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  canvas->Print("FitBe10.pdf");
  gErrorIgnoreLevel = oldLevel;


  inputFile->Close();
}
