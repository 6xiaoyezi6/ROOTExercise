// 三峰 Toy MC：Gaussian(0) + Gaussian(3.368) + Voigtian(10.15) + 独立本底。
// Passing/Failing 同时拟合，三个信号态共用一个自由效率参数。
// 运行：root -l -q 'SimultaneousThreePeaksToy.C()'
// 输出：SimultaneousThreePeaksToy.pdf / .png

#include <iostream>
#include <memory>
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooGaussian.h"
#include "RooVoigtian.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "RooRandom.h"
#include "RooFit.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TString.h"

void SimultaneousThreePeaksToy()
{
    using namespace RooFit;
    RooRandom::randomGenerator()->SetSeed(20261018);
    gStyle->SetOptStat(0);

    // 1. 三个共享信号 PDF：同一个态在 Pass/Fail 中具有相同峰形。
    RooRealVar x("x", "Excitation energy", -5.0, 20.0, "MeV");
    RooRealVar mu0("mu0", "0 MeV mean", 0.0); mu0.setConstant(true);
    RooRealVar mu3("mu3", "3.368 MeV mean", 3.368); mu3.setConstant(true);
    RooRealVar mu10("mu10", "10.15 MeV mean", 10.15); mu10.setConstant(true);
    RooRealVar sigma0("sigma0", "GS resolution", 0.58, 0.25, 1.4);
    RooRealVar sigma3("sigma3", "3.368 resolution", 0.70, 0.25, 1.4);
    RooRealVar sigma10("sigma10", "10.15 resolution", 0.76, 0.25, 1.6);
    RooRealVar gamma10("gamma10", "Voigtian full width", 0.296); gamma10.setConstant(true);
    RooGaussian g0("g0", "ground state", x, mu0, sigma0);
    RooGaussian g3("g3", "3.368 MeV state", x, mu3, sigma3);
    RooVoigtian v10("v10", "10.15 MeV state", x, mu10, gamma10, sigma10);

    // 2. Pass/Fail 的本底有各自的形状与产额。
    RooRealVar slopeP("slopeP", "Passing background slope", -0.25, -0.8, 0.8);
    RooRealVar slopeF("slopeF", "Failing background slope", 0.20, -0.8, 0.8);
    RooChebychev bkgP("bkgP", "Passing background", x, RooArgList(slopeP));
    RooChebychev bkgF("bkgF", "Failing background", x, RooArgList(slopeF));

    // 3. 三个总信号产额 + 共同效率：4 个自由参数代替 6 个独立信号产额。
    RooRealVar n0("n0", "Total GS yield", 650, 0, 20000);
    RooRealVar n3("n3", "Total 3.368 yield", 1400, 0, 20000);
    RooRealVar n10("n10", "Total 10.15 yield", 300, 0, 20000);
    RooRealVar eff("eff", "Common Backtracking efficiency", 0.80, 0.001, 0.999);
    RooFormulaVar n0P("n0P", "@0*@1", RooArgList(eff, n0));
    RooFormulaVar n3P("n3P", "@0*@1", RooArgList(eff, n3));
    RooFormulaVar n10P("n10P", "@0*@1", RooArgList(eff, n10));
    RooFormulaVar n0F("n0F", "(1-@0)*@1", RooArgList(eff, n0));
    RooFormulaVar n3F("n3F", "(1-@0)*@1", RooArgList(eff, n3));
    RooFormulaVar n10F("n10F", "(1-@0)*@1", RooArgList(eff, n10));
    RooRealVar nbP("nbP", "Passing background yield", 650, 0, 30000);
    RooRealVar nbF("nbF", "Failing background yield", 5500, 0, 30000);
    RooAddPdf pdfP("pdfP", "Passing PDF", RooArgList(g0, g3, v10, bkgP), RooArgList(n0P, n3P, n10P, nbP));
    RooAddPdf pdfF("pdfF", "Failing PDF", RooArgList(g0, g3, v10, bkgF), RooArgList(n0F, n3F, n10F, nbF));

    // 4. 先用真值生成两组 Toy 数据，再打乱拟合初值。
    //    真值：eff=0.80；信号总产额分别 650/1400/300；Fail 本底远大于 Pass。
    const int nPassToy = 650 + 1400 + 300;  // 信号总数，仅用于下面固定生成事件数的计算
    const int nP = static_cast<int>(0.8*nPassToy + 650);   // 2530
    const int nF = static_cast<int>(0.2*nPassToy + 5500);  // 5970
    auto dataP = std::unique_ptr<RooDataSet>(pdfP.generate(RooArgSet(x), NumEvents(nP)));
    auto dataF = std::unique_ptr<RooDataSet>(pdfF.generate(RooArgSet(x), NumEvents(nF)));
    n0.setVal(530); n3.setVal(1100); n10.setVal(220); eff.setVal(0.65);
    nbP.setVal(900); nbF.setVal(5000);
    sigma0.setVal(0.75); sigma3.setVal(0.85); sigma10.setVal(0.95);
    slopeP.setVal(0.0); slopeF.setVal(0.0);

    // 5. RooCategory 合并数据，RooSimultaneous 同时拟合两个互斥样本。
    RooCategory category("category", "Pass/Fail category");
    category.defineType("Passing"); category.defineType("Failing");
    RooDataSet combined("combined", "combined sample", RooArgSet(x),
                        Index(category), Import("Passing", *dataP), Import("Failing", *dataF));
    RooSimultaneous sim("sim", "joint PDF", category);
    sim.addPdf(pdfP, "Passing"); sim.addPdf(pdfF, "Failing");
    auto fit = std::unique_ptr<RooFitResult>(sim.fitTo(combined, Extended(true), Save(true), Strategy(1), Hesse(true), PrintLevel(-1)));

    std::cout << "\n===== Simultaneous 3-peak fit =====\n";
    std::cout << "Toy truth: efficiency=0.80, n0=650, n3=1400, n10=300\n";
    std::cout << "Fit efficiency = " << eff.getVal() << " +/- " << eff.getError() << "\n";
    std::cout << "Fit totals: n0=" << n0.getVal() << ", n3=" << n3.getVal() << ", n10=" << n10.getVal() << "\n";
    std::cout << "Passing signal: " << n0P.getVal() << ", " << n3P.getVal() << ", " << n10P.getVal() << "\n";
    std::cout << "Failing signal: " << n0F.getVal() << ", " << n3F.getVal() << ", " << n10F.getVal() << "\n";
    std::cout << "Fit status=" << fit->status() << ", covQual=" << fit->covQual() << "\n";

    // 6. 绘图直接使用各类别的扩展 PDF，避免投影造成的归一化歧义。
    const int bins = 100;  // (-5,20) MeV -> 0.25 MeV/bin
    auto fP = std::unique_ptr<RooPlot>(x.frame(Bins(bins), Title("")));
    auto fF = std::unique_ptr<RooPlot>(x.frame(Bins(bins), Title("")));
    dataP->plotOn(fP.get(), Name("dataP"));
    pdfP.plotOn(fP.get(), LineColor(kRed+1), LineWidth(2), Name("sumP"));
    pdfP.plotOn(fP.get(), Components("g0"), LineColor(kBlue+1), LineStyle(kDashed), Name("g0P"));
    pdfP.plotOn(fP.get(), Components("g3"), LineColor(kMagenta+1), LineStyle(kDashed), Name("g3P"));
    pdfP.plotOn(fP.get(), Components("v10"), LineColor(kOrange+7), LineStyle(kDashed), Name("v10P"));
    pdfP.plotOn(fP.get(), Components("bkgP"), LineColor(kGreen+2), LineStyle(kDotted), Name("bkgCurveP"));
    dataF->plotOn(fF.get(), Name("dataF"));
    pdfF.plotOn(fF.get(), LineColor(kRed+1), LineWidth(2), Name("sumF"));
    pdfF.plotOn(fF.get(), Components("g0"), LineColor(kBlue+1), LineStyle(kDashed), Name("g0F"));
    pdfF.plotOn(fF.get(), Components("g3"), LineColor(kMagenta+1), LineStyle(kDashed), Name("g3F"));
    pdfF.plotOn(fF.get(), Components("v10"), LineColor(kOrange+7), LineStyle(kDashed), Name("v10F"));
    pdfF.plotOn(fF.get(), Components("bkgF"), LineColor(kGreen+2), LineStyle(kDotted), Name("bkgCurveF"));

    TCanvas canvas("canvas", "Passing / Failing 3-peak fit", 950, 790);
    TPad top("top", "", 0, 0.50, 1, 1);
    TPad bottom("bottom", "", 0, 0, 1, 0.50);
    top.SetLeftMargin(0.12); top.SetRightMargin(0.035); top.SetTopMargin(0.07); top.SetBottomMargin(0.02);
    bottom.SetLeftMargin(0.12); bottom.SetRightMargin(0.035); bottom.SetTopMargin(0.02); bottom.SetBottomMargin(0.16);
    top.Draw(); bottom.Draw();

    top.cd();
    fP->SetMinimum(0); fP->GetYaxis()->SetTitle("Events / 0.25 MeV");
    fP->GetXaxis()->SetLabelSize(0); fP->GetXaxis()->SetTitleSize(0);
    fP->Draw();
    TLegend leg(0.73, 0.52, 0.96, 0.89);
    leg.SetBorderSize(0); leg.SetFillStyle(0); leg.SetTextSize(0.030);
    leg.AddEntry(fP->findObject("dataP"), "Toy data", "pe");
    leg.AddEntry(fP->findObject("sumP"), "Total fit", "l");
    leg.AddEntry(fP->findObject("g0P"), "0 MeV Gaussian", "l");
    leg.AddEntry(fP->findObject("g3P"), "3.368 MeV Gaussian", "l");
    leg.AddEntry(fP->findObject("v10P"), "10.15 MeV Voigtian", "l");
    leg.AddEntry(fP->findObject("bkgCurveP"), "Background", "l");
    leg.Draw();
    TLatex latex; latex.SetNDC(); latex.SetTextSize(0.042);
    latex.DrawLatex(0.16, 0.89, "Passing");
    latex.DrawLatex(0.16, 0.80, Form("#epsilon = %.3f #pm %.3f", eff.getVal(), eff.getError()));

    bottom.cd();
    fF->SetMinimum(0); fF->GetYaxis()->SetTitle("Events / 0.25 MeV");
    fF->GetXaxis()->SetTitle("Excitation energy E_{x} (MeV)");
    fF->Draw();
    latex.DrawLatex(0.16, 0.89, "Failing");

    canvas.SaveAs("SimultaneousThreePeaksToy.pdf");
    canvas.SaveAs("SimultaneousThreePeaksToy.png");
}
