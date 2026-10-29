/*
 * 功能：供 ReconsTOGAXSIAlphap.ipynb 通过 subprocess 调用，批量反演靶内能损并导出两体曲线。
 * 方法：复用 HitsLocal 的 ExitDistance、InitialEnergyFromFinal 和 TwoBodyKinematics 接口；
 *       核质量由 NuclearMass 查询。energy 模式读取九列请求，curve 模式输出两条运动学分支。
 * 注意事项：位置 mm、方向无量纲、能量输入输出均为总动能 MeV；内部能损接口为 MeV/u、cm。
 *       沿用半径 15.5 mm、厚度 150 mm 的 LH2 靶及前向 ExitDistance 限制；参数可由 Notebook 传入。
 *       仅修正 LH2 内的出射能损，不新增死层、包壳或穿透修正；失败用具名 -999 和成功标志保存。
 *       外部源码只读包含，不编译安装或写入外部目录；用 ROOT 解释执行本文件，不加 ACLiC 的 +。
 */
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "TGraph.h"
#include "TList.h"
#include "TMultiGraph.h"
#include "TVector3.h"

#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/ExitDistance/ExitDistance.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/EnergyLoss/EnergyLoss.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/EnergyLoss/EnergyLoss.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"

const Double_t kInvalidValue = -999.0;

void WriteKinCurve(std::ofstream& output, TMultiGraph* graphs, Int_t species, Int_t massNumber)
{
  TList* curves = graphs->GetListOfGraphs();
  if(!curves)
    return;
  for(Int_t branch = 0; branch < curves->GetSize(); branch++) {
    TGraph* graph = (TGraph*)curves->At(branch);
    for(Int_t point = 0; point < graph->GetN(); point++) {
      Double_t thetaDeg = 0.0;
      Double_t energyPerU = 0.0;
      graph->GetPoint(point, thetaDeg, energyPerU);
      output << species << ' ' << branch << ' ' << thetaDeg << ' '
             << energyPerU * massNumber << '\n';
    }
  }
}

void KinLoss(const char* mode = "curve", const char* inputName = "",
             const char* outputName = "curve.txt", Double_t beamEnergyPerU = 200.0,
             Double_t thicknessMm = 150.0, Double_t radiusMm = 15.5)
{
  std::ofstream output(outputName);
  if(!output)
    throw std::runtime_error("Cannot open local output table");
  output << std::setprecision(17);

  if(std::string(mode) == "curve") {
    const Double_t alphaMassAmu = NuclearMass::Get("4He").MassAMU;
    const Double_t protonMassAmu = NuclearMass::Get("1H").MassAMU;
    output << "# Species Branch ThetaDeg EnergyMeV\n";
    TMultiGraph* proton = TwoBodyKinematicsEnergyAngleProton(
        beamEnergyPerU, 0.0, alphaMassAmu, protonMassAmu,
        protonMassAmu, alphaMassAmu);
    TMultiGraph* alpha = TwoBodyKinematicsEnergyAngleResidue(
        beamEnergyPerU, 0.0, alphaMassAmu, protonMassAmu,
        protonMassAmu, alphaMassAmu);
    WriteKinCurve(output, proton, 1, 1);
    WriteKinCurve(output, alpha, 4, 4);
    delete proton;
    delete alpha;
    return;
  }

  if(std::string(mode) != "energy")
    throw std::runtime_error("Unknown mode; use energy or curve");
  std::ifstream input(inputName);
  if(!input)
    throw std::runtime_error("Cannot open local energy request table");
  output << "# Entry Species Success PathMm EnergyMeV\n";
  Long64_t entry = 0;
  Int_t species = 0;
  Double_t detectorEnergy = 0.0;
  Double_t x = 0.0;
  Double_t y = 0.0;
  Double_t z = 0.0;
  Double_t dx = 0.0;
  Double_t dy = 0.0;
  Double_t dz = 0.0;
  while(input >> entry >> species >> detectorEnergy >> x >> y >> z >> dx >> dy >> dz) {
    Bool_t success = kFALSE;
    Double_t pathMm = kInvalidValue;
    Double_t energyMeV = kInvalidValue;
    TVector3 direction(dx, dy, dz);
    // 与 HitsLocal 一样，将世界 z 转换为以靶前端为零的坐标。
    const Double_t zTarget = z + thicknessMm * 0.5;
    if((species == 1 || species == 4) && detectorEnergy > 0.0 &&
       std::isfinite(detectorEnergy) && std::isfinite(x) && std::isfinite(y) &&
       std::isfinite(z) && std::isfinite(dx) && std::isfinite(dy) && std::isfinite(dz) &&
       direction.Mag2() > 0.0 && x*x + y*y <= radiusMm*radiusMm &&
       zTarget >= 0.0 && zTarget <= thicknessMm && direction.Theta() < TMath::Pi()/2.0) {
      pathMm = ExitDistance(radiusMm, thicknessMm, x, y, zTarget,
                            direction.Theta(), direction.Phi());
      if(pathMm > 0.0) {
        const Int_t particleZ = species == 1 ? 1 : 2;
        const Int_t particleA = species;
        try {
          const Double_t energyPerU = InitialEnergyFromFinal(
              detectorEnergy / particleA, pathMm * 0.1, particleZ, particleA,
              energy_loss_database::liquid_hydrogen_z,
              energy_loss_database::liquid_hydrogen_a,
              energy_loss_database::liquid_hydrogen_i_ev,
              energy_loss_database::liquid_hydrogen_density, 0.01);
          if(std::isfinite(energyPerU) && energyPerU > 0.0) {
            energyMeV = energyPerU * particleA;
            success = kTRUE;
          }
        }
        catch(const std::exception&) {
          success = kFALSE;
        }
      }
    }
    output << entry << ' ' << species << ' ' << success << ' '
           << pathMm << ' ' << energyMeV << '\n';
  }
  if(!input.eof())
    throw std::runtime_error("Malformed energy request table");
}
