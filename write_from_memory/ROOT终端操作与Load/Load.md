# ROOT 终端使用 `.L` 加载自定义 C++ 文件

## 1. `.L` 支持哪些文件？

| 文件扩展名 | `.L` 加载 | 主要用途 |
| --- | --- | --- |
| `.C` | 支持 | ROOT 宏文件 |
| `.cxx` | 支持 | C++ 实现文件 |
| `.cpp` | 支持 | C++ 实现文件 |
| `.cc` | 支持 | C++ 实现文件 |
| `.h` | 支持 | C/C++ 头文件 |
| `.hh` | 支持 | C++ 头文件 |
| `.hpp` | 支持 | C++ 头文件 |
| `.so` | 支持 | Linux 动态库 |
| `.dylib` | 支持 | macOS 动态库 |

## 2. 加载自定义一维函数

通过 `.L` 加载 `My1DFunctions.h`，使用其中的高斯函数创建 `TF1`。

```cpp
root [0] .L My1DFunctions.h
root [1] TF1 *f1 = new TF1("f1", My1D::Gaussfunction, -10, 10, 3);
root [2] f1->SetParameters(100, 0, 2);
root [3] f1->Draw();
```

## 3. 加载自定义拟合函数

通过 `.L` 加载 `MyVoigtFit.h`，对直方图进行 Gaussian + Voigt 拟合。

```cpp
root [0] .L ../MyVoigtFit.h
root [1] using namespace MyVoigtFit;
root [2] TFile *f = TFile::Open("FitGaussData.root");
root [3] TH1F *h1 = (TH1F *)f->Get("histograms/h1");
root [4] TCanvas *c1 = new TCanvas("c1", "Fit", 800, 600);
root [5] h1->Draw("HIST");
root [6] double maximum = h1->GetMaximum();
root [7] GaussianPeak peak1(Parameter(450, 0, 2*maximum), Parameter(950, 700, 1200), Parameter(70, 0.1, 300));
root [8] VoigtPeak peak2(Parameter(200, 0, 2*maximum), Parameter(1350, 1100, 1600), Parameter(90, 0.1, 300), Parameter(40, 0.1, 300));
root [9] TF1 *fit = FitSingleGaussSingleVoigt(h1, "fit", 600, 1800, peak1, peak2, kTRUE, "R+");
```

拟合输出：

```text
****************************************
Minimizer is Minuit2 / Migrad
Chi2                      =      489.945
NDf                       =          458
Edm                       =  5.07133e-07
NCalls                    =          251
gauss_A                   =      368.396   +/-   2.57178        (limited)
gauss_mean                =      949.818   +/-   0.407366       (limited)
gauss_sigma               =       69.637   +/-   0.318119      (limited)
voigt_A                   =      156.059   +/-   1.53824        (limited)
voigt_mean                =      1349.86   +/-   0.729522       (limited)
voigt_sigma               =      86.8038   +/-   1.28014       (limited)
voigt_gamma               =      6.44319   +/-   3.1389        (limited)
```

## 4. 加载自定义核反应运动学函数

### 4.1 通过 `.L` 加载

通过 `.L` 分别加载 `NuclearMass.cc` 和 `TwoBodyKinematics.cc`，计算并绘制双体核反应运动学曲线。

```cpp
root [1] .L NuclearMass.cc
root [2] .L TwoBodyKinematics.cc
root [3] NuclearMass::Nuclide Be10 = NuclearMass::Get("10Be");// 返回 Nuclide 结构体对象，而不是指针，因此不需要 *
root [4] NuclearMass::Nuclide Proton = NuclearMass::Get("1H");
root [5] Color_t red = kRed + 1;
root [6] SimulatedTree->Draw("ReactionConditions.fRC_Kinetic_Energy[0]:ReactionConditions.fRC_Theta[0]>>h(180,0,90,1000,0,500)","","colz")
Info in <TCanvas::MakeDefCanvas>:  created default TCanvas with name c1
(long long) 99917
root [7] TMultiGraph* graph1 = TwoBodyKinematicsEnergyAngleProton(150,0,Be10.MassAMU,Proton.MassAMU,Proton.MassAMU,Be10.MassAMU,red,2,1,1.0);//注意这种自己定义的函数不用new
root [8] graph1->Draw("L");
root [9]
```

### 4.2 编译到 NPTool 后直接调用

把两体运动学公式编译到 NPTool 中后，可以直接调用。

#### 绘制运动学曲线

```cpp
root [0] NPL::NuclearMass::Nuclide Be10 = NPL::NuclearMass::Get("10Be");
root [1] NPL::NuclearMass::Nuclide proton = NPL::NuclearMass::Get("1H");
root [2] TMultiGraph *graph = NPL::TTwoBodyKinematics::EnergyAngleProton(150.0, 0.0, Be10.MassAMU, proton.MassAMU, proton.MassAMU, Be10.MassAMU, kRed, 2, kSolid, 1.0);
root [3] graph->Draw("AL");
```

#### 计算运动学结果

```cpp
root [0] NPL::NuclearMass::Nuclide Be10 = NPL::NuclearMass::Get("10Be");
root [1] NPL::NuclearMass::Nuclide proton = NPL::NuclearMass::Get("1H");
root [2] NPL::TTwoBodyKinematics::Result r = NPL::TTwoBodyKinematics::Calculate(150.0, 0.0, Be10.MassAMU, proton.MassAMU, proton.MassAMU, Be10.MassAMU, NPL::TTwoBodyKinematics::InputType::ProtonAngle, 65.0, +1);
root [3] r.Theta_c
(double) 65.000000
root [4] r.T_c
(double) 76.178351
root [5] r.Theta_d
(double) 3.7487852
root [6] r.T_d
(double) 1423.8216
```

#### 宏示例

不需要包含头文件即可运行：

```cpp
void TestKinematics()
{
    const Double_t beamEnergyPerU = 150.0;
    const Double_t excitationEnergy = 0.0;

    NPL::NuclearMass::Nuclide Be10 = NPL::NuclearMass::Get("10Be");
    NPL::NuclearMass::Nuclide proton = NPL::NuclearMass::Get("1H");

    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    gStyle->SetOptTitle(0);
    gStyle->SetOptDate(0);

    // 绘图区宽高比为 4:3；上下边距之和与左右边距之和均为 0.20。
    TCanvas *c1 = new TCanvas("c1", "c1", 1000, 750);
    c1->SetCanvasSize(1000, 750);
    c1->SetMargin(0.15, 0.05, 0.15, 0.05);
    c1->SetGrid(0, 0);
    c1->SetTicks(1, 1);

    TMultiGraph *graph = NPL::TTwoBodyKinematics::EnergyAngleProton(
        beamEnergyPerU, excitationEnergy,
        Be10.MassAMU, proton.MassAMU, proton.MassAMU, Be10.MassAMU,
        kRed, 2, kSolid, 1.0);

    graph->SetTitle(";#theta_{p}^{lab} (deg);T_{p}^{lab} (MeV)");
    graph->SetMinimum(0.0);
    graph->SetMaximum(600.0);
    graph->Draw("AL");
    graph->GetXaxis()->SetLimits(0.0, 90.0);
    graph->GetXaxis()->CenterTitle();
    graph->GetYaxis()->CenterTitle();
    graph->GetXaxis()->ChangeLabel(1, -1, 0);
    graph->GetXaxis()->ChangeLabel(-1, -1, 0);

    c1->Modified();
    c1->Update();
}
```
## 3. ROOT 终端绘制两体衰变曲线

在已配置 NPTool 环境的 ROOT 终端中逐行输入以下代码。示例为 `10Be* -> 4He + 6He`，母核动能为 `150 MeV/u`，激发能为 `10.15 MeV`，绘制 `4He` 的实验室系能量—角度曲线；角度相对母核运动方向。

```cpp
NPL::NuclearMass::Nuclide Be10Decay = NPL::NuclearMass::Get("10Be");
NPL::NuclearMass::Nuclide He4 = NPL::NuclearMass::Get("4He");
NPL::NuclearMass::Nuclide He6 = NPL::NuclearMass::Get("6He");

TMultiGraph *graphDecay = NPL::TTwoBodyDecay::EnergyAngleLight(150.0, 10.15,Be10Decay.MassAMU, He4.MassAMU, He6.MassAMU,0.0, 0.0, kRed, 2, kSolid, 1.0);

graphDecay->Draw("AL");
```

横轴为 `4He` 的实验室系角度（deg），纵轴为动能（MeV/u）。两个 `0.0` 分别为 `4He` 和 `6He` 的激发能（MeV）。

## 4. ROOT 终端绘制 10Be(p,p) 反冲质子运动学曲线

以下示例对应 `10Be + p -> p + 10Be` 弹性散射，`10Be` 束流动能为 `150 MeV/u`，末态 `10Be` 的激发能为 `0.0 MeV`。

```cpp
NPL::NuclearMass::Nuclide Be10 = NPL::NuclearMass::Get("10Be");
NPL::NuclearMass::Nuclide proton = NPL::NuclearMass::Get("1H");

TMultiGraph *graphProton = NPL::TTwoBodyKinematics::EnergyAngleProton(150.0, 0.0,Be10.MassAMU, proton.MassAMU, proton.MassAMU, Be10.MassAMU,kRed, 2, kSolid, 1.0);

graphProton->Draw("AL");
```

横轴为反冲质子的实验室系角度（deg），纵轴为动能（MeV/u；质子 A=1，数值也等于 MeV）。质量参数按入射 `10Be`、靶质子、出射质子、末态 `10Be` 的顺序排列。

也可以用更简单的auto写法：
```cpp
auto Be10 = NPL::NuclearMass::Get("10Be");
auto proton = NPL::NuclearMass::Get("1H");

auto graphProton = NPL::TTwoBodyKinematics::EnergyAngleProton(150.0, 0.0, Be10.MassAMU, proton.MassAMU, proton.MassAMU, Be10.MassAMU, kRed, 2, kSolid, 1.0);

graphProton->Draw("AL");


```