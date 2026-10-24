// 演示：一个 Gaussian 信号 + 一阶 Chebychev 本底的 Passing/Failing 联合拟合。
// 运行：root -l -q 'SimultaneousGaussianToy.C()'
// 输出：SimultaneousGaussianToy.pdf、SimultaneousGaussianToy.png
// 无需输入 ROOT 文件；程序自己生成两组 Toy MC 数据。

#include <memory>
#include <iostream>

#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooGaussian.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "RooRandom.h"
#include "RooFit.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TAxis.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TString.h"

void SimultaneousGaussianToy()
{
    using namespace RooFit;
    RooRandom::randomGenerator()->SetSeed(12345);
    gStyle->SetOptStat(0);

    // 1. 共享一个 Gaussian 信号峰，但 Passing / Failing 的本底各自独立。
    RooRealVar x("x", "Excitation energy", -5.0, 5.0, "MeV");
    RooRealVar mean("mean", "signal mean", 0.0, -0.5, 0.5);
    RooRealVar sigma("sigma", "signal sigma", 0.55, 0.2, 1.2);
    RooGaussian signal("signal", "shared Gaussian signal", x, mean, sigma);

    RooRealVar cP("cP", "Passing background slope", -0.20, -0.8, 0.8);
    RooRealVar cF("cF", "Failing background slope",  0.25, -0.8, 0.8);
    RooChebychev bkgP("bkgP", "Passing background", x, RooArgList(cP));
    RooChebychev bkgF("bkgF", "Failing background", x, RooArgList(cF));

    // 2. 两个信号产额被共同的总产额 Nsig 和效率 eff 关联。
    RooRealVar Nsig("Nsig", "total signal yield", 1200, 0, 10000);
    RooRealVar eff("eff", "Backtracking efficiency", 0.75, 0.001, 0.999);
    RooFormulaVar Npass("Npass", "@0*@1", RooArgList(eff, Nsig));
    RooFormulaVar Nfail("Nfail", "(1-@0)*@1", RooArgList(eff, Nsig));

    RooRealVar NbkgP("NbkgP", "Passing background yield", 350, 0, 10000);
    RooRealVar NbkgF("NbkgF", "Failing background yield", 1800, 0, 10000);

    RooAddPdf pdfP("pdfP", "Passing model",
                   RooArgList(signal, bkgP), RooArgList(Npass, NbkgP));
    RooAddPdf pdfF("pdfF", "Failing model",
                   RooArgList(signal, bkgF), RooArgList(Nfail, NbkgF));

    // 3. 用真实参数生成 Toy MC：signal Pass=900，Fail=300；Fail 本底更大。
    auto dataP = std::unique_ptr<RooDataSet>(pdfP.generate(RooArgSet(x), NumEvents(1250)));
    auto dataF = std::unique_ptr<RooDataSet>(pdfF.generate(RooArgSet(x), NumEvents(2100)));

    // 4. 打乱拟合初值，模拟真实分析中的未知参数。
    Nsig.setVal(1000); eff.setVal(0.60);
    NbkgP.setVal(500); NbkgF.setVal(1600);
    mean.setVal(0.15); sigma.setVal(0.75);
    cP.setVal(0.0); cF.setVal(0.0);

    RooCategory sample("sample", "sample");
    sample.defineType("Passing");
    sample.defineType("Failing");
    RooDataSet combined("combined", "Pass and Fail datasets", RooArgSet(x),
                        Index(sample), Import("Passing", *dataP), Import("Failing", *dataF));

    RooSimultaneous simPdf("simPdf", "simultaneous model", sample);
    simPdf.addPdf(pdfP, "Passing");
    simPdf.addPdf(pdfF, "Failing");

    // 5. 一个最大似然拟合同时利用两组数据。
    auto fit = std::unique_ptr<RooFitResult>(
        simPdf.fitTo(combined, Save(true), Extended(true), Strategy(1), PrintLevel(-1))
    );
    std::cout << "\n========== Simultaneous fit ==========" << std::endl;
    std::cout << "Truth: eff = 0.75, total signal = 1200\n";
    std::cout << "Fitted: eff = " << eff.getVal() << " +/- " << eff.getError() << '\n';
    std::cout << "Fitted: Nsig = " << Nsig.getVal() << " +/- " << Nsig.getError() << '\n';
    std::cout << "Signal Pass = " << Npass.getVal()
              << ", Signal Fail = " << Nfail.getVal() << '\n';
    std::cout << "status = " << fit->status()
              << ", covQual = " << fit->covQual() << std::endl;

    // 6. 画两张谱：上下排列、同一横轴范围；曲线按扩展产额归一化。
    auto fP = std::unique_ptr<RooPlot>(x.frame(Bins(50), Title("Passing")));
    auto fF = std::unique_ptr<RooPlot>(x.frame(Bins(50), Title("Failing")));

    combined.plotOn(fP.get(), Cut("sample==sample::Passing"), Name("dataP"));
    simPdf.plotOn(fP.get(), Slice(sample, "Passing"), ProjWData(sample, combined),
                  LineColor(kRed + 1), Name("totalP"));
    simPdf.plotOn(fP.get(), Slice(sample, "Passing"), ProjWData(sample, combined),
                  Components("bkgP"), LineColor(kGreen + 2), LineStyle(kDashed), Name("bkgCurveP"));
    simPdf.plotOn(fP.get(), Slice(sample, "Passing"), ProjWData(sample, combined),
                  Components("signal"), LineColor(kBlue + 1), LineStyle(kDotted), Name("sigCurveP"));

    combined.plotOn(fF.get(), Cut("sample==sample::Failing"), Name("dataF"));
    simPdf.plotOn(fF.get(), Slice(sample, "Failing"), ProjWData(sample, combined),
                  LineColor(kRed + 1), Name("totalF"));
    simPdf.plotOn(fF.get(), Slice(sample, "Failing"), ProjWData(sample, combined),
                  Components("bkgF"), LineColor(kGreen + 2), LineStyle(kDashed), Name("bkgCurveF"));
    simPdf.plotOn(fF.get(), Slice(sample, "Failing"), ProjWData(sample, combined),
                  Components("signal"), LineColor(kBlue + 1), LineStyle(kDotted), Name("sigCurveF"));

    TCanvas canvas("canvas", "Simultaneous fit example", 850, 760);
    TPad top("top", "", 0, 0.49, 1, 1);
    TPad bottom("bottom", "", 0, 0, 1, 0.49);
    top.SetLeftMargin(0.13); top.SetRightMargin(0.04);
    top.SetBottomMargin(0.02); top.SetTopMargin(0.07);
    bottom.SetLeftMargin(0.13); bottom.SetRightMargin(0.04);
    bottom.SetBottomMargin(0.17); bottom.SetTopMargin(0.02);
    top.Draw(); bottom.Draw();

    top.cd();
    fP->SetMinimum(0); fP->GetYaxis()->SetTitle("Events / bin");
    fP->GetXaxis()->SetLabelSize(0); fP->GetXaxis()->SetTitleSize(0);
    fP->Draw();
    TLegend legend(0.69, 0.65, 0.94, 0.89);
    legend.SetBorderSize(0); legend.SetFillStyle(0);
    legend.AddEntry(fP->findObject("dataP"), "Toy data", "pe");
    legend.AddEntry(fP->findObject("totalP"), "Total fit", "l");
    legend.AddEntry(fP->findObject("sigCurveP"), "Gaussian", "l");
    legend.AddEntry(fP->findObject("bkgCurveP"), "Background", "l");
    legend.Draw();
    TLatex label;
    label.SetNDC(); label.SetTextSize(0.050);
    label.DrawLatex(0.18, 0.83, Form("#epsilon = %.3f #pm %.3f", eff.getVal(), eff.getError()));

    bottom.cd();
    fF->SetMinimum(0); fF->GetYaxis()->SetTitle("Events / bin");
    fF->GetXaxis()->SetTitle("Excitation energy E_{x} (MeV)");
    fF->Draw();

    canvas.SaveAs("SimultaneousGaussianToy.pdf");
    canvas.SaveAs("SimultaneousGaussianToy.png");
}
