/*
 * 功能：在 x ∈ [0, 2] 的显示范围内生成 21 个带高斯扰动的三次函数数据点，
 *       对数据分别做三次多项式最小二乘和 RooFit 最大似然拟合，并将结果保存到
 *       GenerateData.root；将数据点、真实函数、两条拟合曲线及各自的 95% 置信带
 *       保存到 GenerateData.pdf，并用六项图例区分数据、真实函数、两种拟合及置信带。
 * 方法：x 按 0.1 的步长从 0 取到 2.0，并包含两个端点；
 *       y 由 0.2x^3+0.5x^2-x+0.5 加上
 *       TRandom3 产生的 N(0, 0.3) 随机数得到；TGraphErrors 的 x 误差为 0，y 误差为 0.3。
 *       在 [0, 2] 范围内用 y=[0]+[1]x+[2]x^2+[3]x^3 做加权最小二乘拟合，
 *       在 [0, 2] 内取 100 个等距点，用 TVirtualFitter 计算并绘制平滑的 95% 置信带。
 *       RooFit 使用条件高斯模型 y|x~N(mu(x),sigma)，mu(x) 为三次多项式，sigma 自由拟合；
 *       RooFitResult 通过 VisualizeError 绘制 1.96 sigma 的绿色 95% 置信带。
 *       真实函数用透明度为 0.3 的红线绘制，最小二乘和最大似然拟合函数分别用蓝线和绿色虚线绘制。
 * 注意事项：TRandom3 使用固定种子 123456，每次运行可重现同一组数据；
 *           置信带表示拟合函数均值的不确定度，不是单个新观测值的预测带。
 */

//三次多项式不是“最大似然拟合的目标函数”，它是概率模型里的均值函数。真正被最大化的目标函数是似然L，也就是所有数据点对应概率密度的乘积。
//同方差高斯误差下，最大似然估计都回归系数与最小二乘法完全相同！

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TRandom3.h>
#include <TColor.h>
#include <TVirtualFitter.h>

#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooCurve.h>
#include <RooDataSet.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooGaussian.h>
#include <RooPlot.h>
#include <RooPolyVar.h>
#include <RooRealVar.h>

void GenerateData() {
  constexpr Double_t xMin = 0.0;
  constexpr Double_t xMax = 2.0;
  constexpr Int_t pointCount = 21;
  constexpr Double_t xStep = 0.1;
  constexpr Double_t errorMean = 0.0;
  constexpr Double_t errorSigma = 0.3;

  Double_t x[pointCount] = {0.0};
  Double_t y[pointCount] = {0.0};
  Double_t xError[pointCount] = {0.0};
  Double_t yError[pointCount] = {0.0};

  TRandom3 random(123456);
  for (Int_t i = 0; i < pointCount; ++i) {
    x[i] = i * xStep;
    const Double_t trueY = 0.2 * x[i] * x[i] * x[i]
                         + 0.5 * x[i] * x[i] - x[i] + 0.5;
    y[i] = trueY + random.Gaus(errorMean, errorSigma);
    yError[i] = errorSigma;
  }

  TGraphErrors *g1 = new TGraphErrors(pointCount, x, y, xError, yError);
  g1->SetName("g1");
  g1->SetTitle("Random data with Gaussian errors;x;y");
  g1->SetMarkerStyle(20);
  g1->SetMarkerSize(0.9);
  g1->SetMarkerColor(kAzure - 4);
  g1->SetLineColor(kAzure - 4);

  TF1 *f1 = new TF1("f1", "0.2*x*x*x+0.5*x*x-x+0.5", xMin, xMax);
  f1->SetLineColorAlpha(kRed, 0.3);
  f1->SetLineWidth(2);

  TF1 *f2 = new TF1("f2", "pol3", xMin, xMax);
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(2);
  g1->Fit(f2, "RS0N");

  constexpr Int_t confidencePointCount = 100; //100个置信带的计算数据点
  TGraphErrors *grint = new TGraphErrors(confidencePointCount);
  grint->SetName("grint");
  grint->SetTitle("95% confidence band");
  for (Int_t i = 0; i < confidencePointCount; ++i) {
    const Double_t confidenceX =
        xMin + (xMax - xMin) * i / (confidencePointCount - 1);
    grint->SetPoint(i, confidenceX, 0.0);
  }
  TVirtualFitter::GetFitter()->GetConfidenceIntervals(grint, 0.95);//95%的置信带
  grint->SetFillColorAlpha(kBlue, 0.2);
  grint->SetLineColor(kBlue);

  RooRealVar rooX("rooX", "x", xMin, xMax);
  RooRealVar rooY("rooY", "y", -10.0, 10.0);
  RooRealVar rooA0("rooA0", "a0", f2->GetParameter(0), -10.0, 10.0);
  RooRealVar rooA1("rooA1", "a1", f2->GetParameter(1), -10.0, 10.0);
  RooRealVar rooA2("rooA2", "a2", f2->GetParameter(2), -10.0, 10.0);
  RooRealVar rooA3("rooA3", "a3", f2->GetParameter(3), -10.0, 10.0);
  RooPolyVar rooMean("rooMean", "RooFit cubic mean", rooX,
                     RooArgList(rooA0, rooA1, rooA2, rooA3));
  RooRealVar rooSigma("rooSigma", "Gaussian sigma", errorSigma, 0.05, 2.0);
  RooGaussian model("model", "Gaussian y given x", rooY, rooMean, rooSigma);

  RooArgSet rooObservables(rooX, rooY);
  RooDataSet data("data", "x-y data", rooObservables);
  for (Int_t i = 0; i < pointCount; ++i) {
    rooX.setVal(x[i]);
    rooY.setVal(y[i]);
    data.add(rooObservables);
  }

  RooFitResult *fitResult = model.fitTo(
      data, RooFit::ConditionalObservables(rooX), RooFit::Save(),
      RooFit::PrintLevel(-1));

  const Double_t confidenceZ = 1.95996398454005;//标准正态分布对应双侧 95% 置信区间的临界值
  const Int_t greenBandColor = TColor::GetColorTransparent(kGreen + 1, 0.2);
  RooPlot *frame = rooX.frame(RooFit::Range(xMin, xMax), RooFit::Bins(100));
  data.plotOnXY(frame, RooFit::YVar(rooY));
  rooMean.plotOn(frame, RooFit::VisualizeError(*fitResult, confidenceZ),
                 RooFit::FillColor(greenBandColor),
                 RooFit::Name("rooConfidenceBand"));
  rooMean.plotOn(frame, RooFit::LineColor(kGreen + 2),
                 RooFit::LineStyle(kDashed), RooFit::LineWidth(2),
                 RooFit::Name("rooFitCurve"));
  RooCurve *rooConfidenceBand = frame->getCurve("rooConfidenceBand");
  RooCurve *rooFitCurve = frame->getCurve("rooFitCurve");

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
  c1->SetGrid(0, 0); c1->Modified(); c1->Update();
  g1->Draw("AP");
  g1->GetXaxis()->SetLimits(xMin, xMax);
  grint->Draw("E3 SAME");
  rooConfidenceBand->Draw("F SAME");
  f1->Draw("L SAME");
  f2->Draw("L SAME");
  rooFitCurve->Draw("L SAME");
  g1->Draw("P SAME");

  TLegend *legend = new TLegend(0.14, 0.67, 0.53, 0.88);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->AddEntry(g1, "Gaussian data", "lep");
  legend->AddEntry(f1, "True function", "l");
  legend->AddEntry(f2, "Weighted least squares", "l");
  legend->AddEntry(grint, "WLS 95% confidence band", "f");
  legend->AddEntry(rooFitCurve, "Gaussian maximum likelihood", "l");
  legend->AddEntry(rooConfidenceBand, "Gaussian MLE 95% confidence band", "f");
  legend->Draw();
  c1->SaveAs("GenerateData.pdf");

  TFile *outputFile = new TFile("GenerateData.root", "RECREATE");
  g1->Write();
  f1->Write();
  f2->Write();
  grint->Write();
  fitResult->Write("rooFitResult");
  frame->Write("rooFrame");
  rooConfidenceBand->Write("rooConfidenceBand");
  rooFitCurve->Write("rooFitCurve");
  c1->Write();
  outputFile->Close();
}
