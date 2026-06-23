#include <iostream>

#include "TCanvas.h"
#include "TF1.h"
#include "TMath.h"

//ROOT中定义函数的方法 (变量，可变参数（系数）)
Double_t Gaussfunction(Double_t *x, Double_t *par)
{
   return TMath::Gaus(x[0], par[0], par[1], true);
}

Double_t polynominal(Double_t *x, Double_t *par)
{
    return par[0]+ par[1] * x[0]+ par[2] * TMath::Power(x[0], 2)+ par[3] * TMath::Power(x[0], 3);
}



void NumericalIntegration()
{
   Double_t u = 0.0;
   Double_t sigma = 1.0;
   
   //积分范围
   Double_t xmin = u - sigma;
   Double_t xmax = u + sigma;


   //高斯函数
   // TF1 *f = new TF1("f", Gaussfunction, -5, 5, 2); //（函数名，函数表达式，画图的xy范围，自由参数的个数）
   // f->SetParameters(u, sigma);
   // f->SetMinimum(0.0);

   //三次多项式
   TF1 *f = new TF1("f",polynominal,-2,2,4);
   f->SetParameters(1,2,3,4);


   std::cout <<"Integral Interval =[" << xmin<< ","<< xmax <<"]  "<< "Integral = " << f->Integral(xmin, xmax) << std::endl;

   TCanvas *c1 = new TCanvas("c1", "c1", 1000, 800);
   f->Draw();

   //在积分区间内填色
   TF1 *hFill = (TF1*)f->DrawCopy("SAME");
   hFill->SetRange(xmin, xmax);
   hFill->SetFillColor(kBlue+1);
   hFill->SetFillStyle(3001);
   hFill->SetLineColor(kBlue+1);
   hFill->SetBit(kCanDelete);
   hFill->Draw("F SAME");

   f->Draw("SAME");
   c1->SaveAs("NumericalIntegration_function.svg");
}
