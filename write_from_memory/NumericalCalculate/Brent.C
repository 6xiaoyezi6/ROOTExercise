#include <iostream>

#include "Math/Functor.h"
#include "Math/RootFinder.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TF1.h"

//源码的位置
///Users/yemingxin/root_source_code/math/mathcore/inc/Math/RootFinder.h:77
///Users/yemingxin/root_source_code/math/mathcore/src/RootFinder.cxx:50
///Users/yemingxin/root_source_code/math/mathcore/inc/Math/BrentRootFinder.h:51
///Users/yemingxin/root_source_code/math/mathcore/src/BrentMethods.cxx:130

//ROOT找根和ROOT画图对函数的结构要求不一样

// TF1 画图用
Double_t fun(Double_t *x, Double_t *par)
{
   return TMath::Sin(x[0]) + TMath::Cos(x[0]);
}

//RootFinder 找根用
Double_t f(Double_t x)   
{
   return TMath::Sin(x) + TMath::Cos(x);
}

void Brent()
{
   ROOT::Math::RootFinder solver(ROOT::Math::RootFinder::kBRENT);
   ROOT::Math::Functor1D func(&f); //把这个普通函数包装成 ROOT Math 可以识别的一维函数对象

   solver.SetFunction(func, 0.0, TMath::Pi());
   bool ok = solver.Solve(100, 1e-12, 1e-12);//RootFinder的Solve函数，solver.Solve(最大迭代次数, 绝对误差, 相对误差)

   if (!ok) {
      std::cout << "Root finding failed." << std::endl;
      return;
   }

   Double_t root = solver.Root();
   std::cout << "root = " << root << std::endl;
   std::cout << "f(root) = " << f(root) << std::endl;

   //----------------------------------------------------------
   TCanvas*c1 = new TCanvas("c1","c1",1000,800);
   TF1 *fplot = new TF1("fplot", fun, 0.0, 4.0 * TMath::Pi(), 0);
   fplot->SetLineWidth(2);
   fplot->Draw();
   c1->SaveAs("Brent_function.svg");
}
