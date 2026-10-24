/*
 * 功能：向 ForExperiment_output 导出实验能角对照图使用的两体运动学曲线和参数记录。
 * 方法：与 PictureForThesis/Draw_8.C 相同，计算 10Be(p,p)10Be、150 MeV/u、
 *       Ex=0、3.368、6.179、10.15 MeV；质量通过 NuclearMass 查询，调用现有
 *       TwoBodyKinematicsEnergyAngleProton，保留每条分支的点顺序及 ROOT 原色 RGB。
 * 注意事项：不读取实验数据、不加载模型、不训练；角度 deg、质子动能 MeV。
 *       质子质量数为 1，接口的质子 MeV/u 数值即总动能 MeV。
 *       需要本地 ROOT、TwoBodyKinematics、NuclearMass 和 nubtab20.asc。
 *       运行覆盖本宏所在目录下 ForExperiment_output 中的同名 CSV/JSON，不依赖启动目录。
 * 运行：root -l -b -q ForExperimentKinematics.C；ForExperiment.ipynb 也会调用本宏。
 */
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.cc"
#include "TColor.h"
#include "TGraph.h"
#include "TList.h"
#include "TMultiGraph.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace ForExperimentKinematicsInternal {
const Double_t kBeamEnergyPerU = 150.0;
const Double_t kExcitationEnergy[4] = {0.0, 3.368, 6.179, 10.15};
const Color_t kLineColor[4] = {kRed + 1, kBlue + 1, kGreen + 2, kMagenta + 1};
const Float_t kLineAlpha = 0.4;
}

void ForExperimentKinematics()
{
  using namespace ForExperimentKinematicsInternal;
  const TString outputDirectory = TString(gSystem->DirName(__FILE__)) + "/ForExperiment_output";
  gSystem->mkdir(outputDirectory.Data(), kTRUE);
  const Double_t mass10BeAmu = NuclearMass::Get("10Be").MassAMU;
  const Double_t massProtonAmu = NuclearMass::Get("1H").MassAMU;

  std::ofstream curves((outputDirectory + "/KinematicsCurves.csv").Data());
  if (!curves) throw std::runtime_error("Cannot create KinematicsCurves.csv");
  curves << "StateIndex,Ex_MeV,CurveBranch,PointIndex,Theta_deg,ProtonEnergy_MeV,Red,Green,Blue\n";
  curves << std::setprecision(17);
  Long64_t savedPoints = 0;
  for (Int_t state = 0; state < 4; ++state) {
    TMultiGraph* multiGraph = TwoBodyKinematicsEnergyAngleProton(
        kBeamEnergyPerU, kExcitationEnergy[state],
        mass10BeAmu, massProtonAmu, massProtonAmu, mass10BeAmu,
        kLineColor[state], 1, kSolid, kLineAlpha);
    TColor* color = gROOT->GetColor(kLineColor[state]);
    if (!color) throw std::runtime_error("ROOT reference line color is unavailable");
    for (Int_t branch = 0; branch < multiGraph->GetListOfGraphs()->GetSize(); ++branch) {
      TGraph* graph = (TGraph*)multiGraph->GetListOfGraphs()->At(branch);
      for (Int_t point = 0; point < graph->GetN(); ++point) {
        Double_t theta = 0.0, energy = 0.0;
        graph->GetPoint(point, theta, energy);
        if (!std::isfinite(theta) || !std::isfinite(energy)) continue;
        curves << state << ',' << kExcitationEnergy[state] << ',' << branch << ',' << point << ','
               << theta << ',' << energy << ',' << color->GetRed() << ',' << color->GetGreen() << ',' << color->GetBlue() << '\n';
        ++savedPoints;
      }
    }
    delete multiGraph;
  }
  curves.close();
  if (!curves || savedPoints == 0) throw std::runtime_error("Kinematic curve export failed");

  std::ofstream metadata((outputDirectory + "/KinematicsConfiguration.json").Data());
  if (!metadata) throw std::runtime_error("Cannot create KinematicsConfiguration.json");
  metadata << std::setprecision(17)
           << "{\n  \"reaction\": \"10Be(p,p)10Be\",\n"
           << "  \"reference_macro\": \"/Users/yemingxin/RIKEN_Inelastic_Be/ppBe10/PictureForThesis/Draw_8.C\",\n"
           << "  \"beam_energy_per_u_MeV\": " << kBeamEnergyPerU << ",\n"
           << "  \"excitation_MeV\": [0.0, 3.368, 6.179, 10.15],\n"
           << "  \"mass_10Be_u\": " << mass10BeAmu << ",\n"
           << "  \"mass_proton_u\": " << massProtonAmu << ",\n"
           << "  \"mass_table\": \"" << NuclearMass::DefaultTablePath << "\",\n"
           << "  \"angle_unit\": \"deg\", \"proton_energy_unit\": \"MeV\",\n"
           << "  \"line_alpha\": " << kLineAlpha << ", \"line_width\": 1, \"line_style\": \"solid\",\n"
           << "  \"points\": " << savedPoints << "\n}\n";
  metadata.close();
  if (!metadata) throw std::runtime_error("Kinematic parameter export failed");
  std::cout << "Kinematics: 150 MeV/u, Ex=0/3.368/6.179/10.15 MeV; points=" << savedPoints
            << "; output=" << outputDirectory << std::endl;
}
