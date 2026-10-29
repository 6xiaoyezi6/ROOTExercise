.L 支持哪些文件？
文件扩展名	.L 加载	主要用途
.C	支持	ROOT 宏文件
.cxx	支持	C++ 实现文件
.cpp	支持	C++ 实现文件
.cc	支持	C++ 实现文件
.h	支持	C/C++ 头文件
.hh	支持	C++ 头文件
.hpp	支持	C++ 头文件
.so	支持	Linux 动态库
.dylib	支持	macOS 动态库



root [0] .L My1DFunctions.h
root [1] TF1 *f1 = new TF1("f1", My1D::Gaussfunction, -10, 10, 3);
root [2] f1->SetParameters(100, 0, 2);
root [3] f1->Draw();






root [0] .L ../MyVoigtFit.h
root [1] using namespace MyVoigtFit;
root [2] TFile *f = TFile::Open("FitGaussData.root");
root [3] TH1F *h1 = (TH1F *)f->Get("histograms/h1");
root [4] TCanvas *c1 = new TCanvas("c1", "Fit", 800, 600);
root [5] h1->Draw("HIST");
root [6] double maximum = h1->GetMaximum();
root [7] GaussianPeak peak1(Parameter(450, 0, 2*maximum), Parameter(950, 700, 1200), Parameter(70, 0.1, 300));
root [8] VoigtPeak peak2(Parameter(200, 0, 2*maximum), Parameter(1350, 1100, 1600), Parameter(90, 0.1, 300), Parameter(40, 0.1, 300));
root [9]
root [9] TF1 *fit = FitSingleGaussSingleVoigt(h1, "fit", 600, 1800, peak1, peak2, kTRUE, "R+");
****************************************
Minimizer is Minuit2 / Migrad
Chi2                      =      489.945
NDf                       =          458
Edm                       =  5.07133e-07
NCalls                    =          251
gauss_A                   =      368.396   +/-   2.57178      	 (limited)
gauss_mean                =      949.818   +/-   0.407366     	 (limited)
gauss_sigma               =       69.637   +/-   0.318119     	 (limited)
voigt_A                   =      156.059   +/-   1.53824      	 (limited)
voigt_mean                =      1349.86   +/-   0.729522     	 (limited)
voigt_sigma               =      86.8038   +/-   1.28014      	 (limited)
voigt_gamma               =      6.44319   +/-   3.1389       	 (limited)
root [10]