/*
 * 功能：在 x ∈ [0, 2] 上生成 21 个 Poisson 低计数点，比较加权最小二乘与
 *       Poisson 最大似然的三次多项式拟合曲线和 95% 逐点置信区间带，
 *       输出 GenerateData2.root/PDF。
 * 方法：代码开头的缩放系数 K 控制整体计数强度；x 按 0.1
 *       等距取点并包含左右端点，N_i~Poisson(mu_i)，其中
 *       mu=K*(0.2*x^3+0.5*x^2-x+0.5)；两种拟合均使用直接三次多项式均值，
 *       chi2 误差取 sqrt(max(N,1))，RooFit 使用条件 Poisson 似然；
 *       置信带由参数协方差矩阵线性传播得到；终端系数表中的 True 列
 *       自动显示经过 K 缩放的真实系数，便于与两种拟合结果直接比较。
 * 注意事项：K 必须大于 0；固定随机种子保证可重现；置信带不是预测带或同时置信带；
 *           低计数下 sqrt(max(N,1)) 只是 Gaussian chi2 近似；RooFit 内部将
 *           非正的三次多项式均值截到 1e-9，保证 Poisson 似然与绿色曲线均有效；
 *           画布显式关闭横纵网格。
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TMatrixDSym.h>
#include <TRandom3.h>
#include <TStyle.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooFormulaVar.h>
#include <RooPoisson.h>
#include <RooRealVar.h>

void GenerateData2() {
  constexpr Double_t K = 1;// K倍的均值，均值越大，抽到y=0的概率越小
  constexpr Double_t xMin = 0.0, xMax = 2.0;
  constexpr Int_t pointCount = 21;
  constexpr Double_t xStep = 0.1;
  constexpr Int_t confidencePointCount = 200;
  constexpr Double_t confidenceZ = 1.95996398454005;
  constexpr Double_t trueA0 = 0.5, trueA1 = -1.0;
  constexpr Double_t trueA2 = 0.5, trueA3 = 0.2;
  Double_t x[pointCount] = {0.0}, y[pointCount] = {0.0};
  Double_t xError[pointCount] = {0.0}, yError[pointCount] = {0.0};
  Double_t expectedTotal = 0.0, observedTotal = 0.0;

  TRandom3 random(123456);
  std::cout << "\nGenerated Poisson data\n";
  std::cout << " i      x       mu_true      N       sigma_for_chi2\n";
  for (Int_t i = 0; i < pointCount; ++i) {
    x[i] = i * xStep;
    const Double_t trueMu = K * (trueA0 + trueA1 * x[i] + trueA2 * x[i] * x[i]
                               + trueA3 * x[i] * x[i] * x[i]);
    y[i] = random.PoissonD(trueMu);
    yError[i] = std::sqrt(std::max(1.0, y[i]));
    expectedTotal += trueMu;
    observedTotal += y[i];
    std::cout << Form("%2d   %5.2f   %10.4f   %4.0f   %10.4f\n",
                      i, x[i], trueMu, y[i], yError[i]);
  }
  std::cout << Form("K = %.3f, expected total = %.3f, generated total = %.0f\n",
                    K, expectedTotal, observedTotal);

  TGraphErrors *g1 = new TGraphErrors(pointCount, x, y, xError, yError);
  g1->SetName("g1"); g1->SetTitle("Poisson low-count data;x;Counts");
  g1->SetMarkerStyle(20); g1->SetMarkerSize(0.9);
  g1->SetMarkerColor(kBlack); g1->SetLineColor(kBlack);
  TF1 *fTrue = new TF1("fTrue", "pol3", xMin, xMax);
  fTrue->SetParameters(K * trueA0, K * trueA1, K * trueA2, K * trueA3);
  fTrue->SetLineColorAlpha(kRed, 0.45); fTrue->SetLineWidth(3);

  TF1 *fChi2 = new TF1("fChi2", "pol3", xMin, xMax);
  fChi2->SetParameters(K * trueA0, K * trueA1, K * trueA2, K * trueA3);
  fChi2->SetParNames("a0", "a1", "a2", "a3");
  fChi2->SetLineColor(kBlue); fChi2->SetLineWidth(2);
  TFitResultPtr chi2Result = g1->Fit(fChi2, "RS0N");
  const TMatrixDSym chi2Cov = chi2Result->GetCovarianceMatrix();

  constexpr Double_t scaleForLimits = std::max(1.0, K);
  constexpr Double_t coefficientLimit = 10.0 * scaleForLimits;
  RooRealVar rooX("rooX", "x", xMin, xMax);
  RooRealVar rooN("rooN", "N", 0.0, 100.0 * scaleForLimits);
  RooRealVar rooA0("rooA0", "a0", fChi2->GetParameter(0),
                   -coefficientLimit, coefficientLimit);
  RooRealVar rooA1("rooA1", "a1", fChi2->GetParameter(1),
                   -coefficientLimit, coefficientLimit);
  RooRealVar rooA2("rooA2", "a2", fChi2->GetParameter(2),
                   -coefficientLimit, coefficientLimit);
  RooRealVar rooA3("rooA3", "a3", fChi2->GetParameter(3),
                   -coefficientLimit, coefficientLimit);
  RooArgList meanArgs(rooA0, rooA1, rooA2, rooA3, rooX);
  RooFormulaVar rooMean(
      "rooMean", "max(1e-9, @0 + @1*@4 + @2*@4*@4 + @3*@4*@4*@4)", meanArgs);
  RooPoisson poissonModel("poissonModel", "N given x", rooN, rooMean);
  RooArgSet observables(rooX, rooN);
  RooDataSet data("data", "Poisson x-N data", observables);
  for (Int_t i = 0; i < pointCount; ++i) {
    rooX.setVal(x[i]); rooN.setVal(y[i]); data.add(observables);
  }
  RooFitResult *mleResult = poissonModel.fitTo(
      data, RooFit::ConditionalObservables(rooX), RooFit::Save(), RooFit::PrintLevel(-1));

  TF1 *fMLE = new TF1(
      "fMLE", "max(1e-9, [0]+[1]*x+[2]*x*x+[3]*x*x*x)", xMin, xMax);
  fMLE->SetParameters(rooA0.getVal(), rooA1.getVal(), rooA2.getVal(), rooA3.getVal());
  fMLE->SetParNames("a0", "a1", "a2", "a3");
  fMLE->SetLineColor(kGreen + 2); fMLE->SetLineStyle(kDashed); fMLE->SetLineWidth(3);
  RooArgList mleParameters(rooA0, rooA1, rooA2, rooA3);
  const TMatrixDSym mleCov = mleResult->reducedCovarianceMatrix(mleParameters);

  TGraphErrors *chi2Band = new TGraphErrors(confidencePointCount);
  chi2Band->SetName("chi2Band");
  chi2Band->SetTitle("Weighted least-squares 95% pointwise confidence band");
  chi2Band->SetFillColorAlpha(kBlue, 0.20); chi2Band->SetLineColor(kBlue);
  TGraphErrors *mleBand = new TGraphErrors(confidencePointCount);
  mleBand->SetName("mleBand"); mleBand->SetTitle("Poisson MLE 95% pointwise confidence band");
  mleBand->SetFillColorAlpha(kGreen + 1, 0.20); mleBand->SetLineColor(kGreen + 2);

  auto FillConfidenceBand = [&](TGraphErrors *band, TF1 *fitFunction,
                                const TMatrixDSym &covariance,
                                Bool_t protectPoissonMean) {
    for (Int_t i = 0; i < confidencePointCount; ++i) {
      const Double_t xx = xMin + (xMax - xMin) * i / (confidencePointCount - 1);
      const Double_t basis[4] = {1.0, xx, xx * xx, xx * xx * xx};
      Double_t rawMu = 0.0;
      for (Int_t a = 0; a < 4; ++a) rawMu += fitFunction->GetParameter(a) * basis[a];
      const Bool_t meanIsClipped = protectPoissonMean && rawMu <= 1e-9;
      const Double_t mu = meanIsClipped ? 1e-9 : rawMu;
      Double_t gradient[4] = {0.0}, variance = 0.0;
      for (Int_t a = 0; a < 4; ++a)
        gradient[a] = meanIsClipped ? 0.0 : basis[a];
      for (Int_t a = 0; a < 4; ++a)
        for (Int_t b = 0; b < 4; ++b)
          variance += gradient[a] * covariance(a, b) * gradient[b];
      band->SetPoint(i, xx, mu);
      band->SetPointError(i, 0.0, confidenceZ * std::sqrt(std::max(0.0, variance)));
    }
  };
  FillConfidenceBand(chi2Band, fChi2, chi2Cov, kFALSE);
  FillConfidenceBand(mleBand, fMLE, mleCov, kTRUE);

  std::cout << "\n================ Weighted least squares ================\n";
  chi2Result->Print("V");
  std::cout << "\n================ Poisson maximum likelihood ============\n";
  mleResult->Print("V");
  std::cout << "\nBest-fit coefficients\n";
  std::cout << "              True              chi2/WLS             Poisson MLE\n";
  std::cout << Form("a0      %14.6f      %14.6f      %14.6f\n",
                    K * trueA0, fChi2->GetParameter(0), rooA0.getVal());
  std::cout << Form("a1      %14.6f      %14.6f      %14.6f\n",
                    K * trueA1, fChi2->GetParameter(1), rooA1.getVal());
  std::cout << Form("a2      %14.6f      %14.6f      %14.6f\n",
                    K * trueA2, fChi2->GetParameter(2), rooA2.getVal());
  std::cout << Form("a3      %14.6f      %14.6f      %14.6f\n",
                    K * trueA3, fChi2->GetParameter(3), rooA3.getVal());

  const Bool_t previousGridX = gStyle->GetPadGridX();
  const Bool_t previousGridY = gStyle->GetPadGridY();
  gStyle->SetPadGridX(kFALSE); gStyle->SetPadGridY(kFALSE);
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  gStyle->SetPadGridX(previousGridX); gStyle->SetPadGridY(previousGridY);
  g1->Draw("AP"); g1->GetXaxis()->SetLimits(xMin, xMax); g1->SetMinimum(0.0);
  chi2Band->Draw("E3 SAME"); mleBand->Draw("E3 SAME");
  fTrue->Draw("L SAME"); fChi2->Draw("L SAME"); fMLE->Draw("L SAME"); g1->Draw("P SAME");
  TLegend *legend = new TLegend(0.14, 0.67, 0.53, 0.88);
  legend->SetBorderSize(0); legend->SetFillStyle(0);
  legend->AddEntry(g1, "Poisson data", "lep");
  legend->AddEntry(fTrue, "True mean", "l");
  legend->AddEntry(fChi2, "Weighted least squares", "l");
  legend->AddEntry(chi2Band, "WLS 95% confidence band", "f");
  legend->AddEntry(fMLE, "Poisson maximum likelihood", "l");
  legend->AddEntry(mleBand, "Poisson MLE 95% confidence band", "f");
  legend->Draw();
  c1->SetGrid(0, 0); c1->Modified(); c1->Update();
  c1->SaveAs("GenerateData2.pdf");

  TFile *outputFile = new TFile("GenerateData2.root", "RECREATE");
  g1->Write(); fTrue->Write(); fChi2->Write(); fMLE->Write();
  chi2Band->Write(); mleBand->Write(); chi2Result->Write("chi2FitResult");
  mleResult->Write("poissonMLEFitResult"); data.Write("rooDataSet"); c1->Write();
  outputFile->Close();
}
