#include "RtypesCore.h"
#include "TDecompLU.h"
#include "TMatrixD.h"
#include "TMatrixDfwd.h"
#include "TVector3.h"
#include "TVectorD.h"
#include <iostream>

#include "Math/Functor.h"
#include "Math/RootFinder.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TMath.h"

void TestBackTracking() {
  // 第一条直线: r = r1 + t*n1
  TVector3 r1(0, 0, 0);
  TVector3 n1(1, 0, 0);

  // 第二条直线: r = r2 + s*n2
  TVector3 r2(0, 0, 1);
  TVector3 n2(0, 1, 0);

  TVector3 minr0; // 到两条直线最短的点坐标
  Double_t d;     // 点到两条空间直线的距离平方和开根号

  // 解矩阵方程 Ax=b
  TMatrixD A(2, 2);
  A(0, 0) = n1.Mag2(); // 模长平方
  A(0, 1) = -n1.Dot(n2);
  A(1, 0) = n1.Dot(n2);
  A(1, 1) = -n2.Mag2();

  TVectorD b(2);
  b(0) = -(r1 - r2).Dot(n1);
  b(1) = -(r1 - r2).Dot(n2);

  TDecompLU lu(A);
  Bool_t ok;
  TVectorD x = lu.Solve(b, ok);

  minr0 = (r1 + x(0) * n1 + r2 + x(1) * n2) * 0.5;

  Double_t d1_2 = (minr0 - r1).Cross(n1).Mag2() / n1.Mag2();
  Double_t d2_2 = (minr0 - r2).Cross(n2).Mag2() / n2.Mag2();

  d = TMath::Sqrt(d1_2 + d2_2);

  std::cout << "r0 = " << minr0.X() << "  " << minr0.Y() << "  " << minr0.Z()
            << std::endl;
  std::cout << "d = " << d << std::endl;
}
