// 功能：逐事件计算原重建与 ADC MLP 两种质子动能对应的 10Be 激发能。
// 方法：NuclearMass 的 MassAMU（u）质量和 TwoBodyKinematicsMissingMass；原样返回 SourceEntry、espriSide。
// 注意事项：输入为实验同事件角度及两种能量；束流总动能 MeV，角度 deg；失败值 -999 配合有效标志。
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/TwoBodyKinematics/TwoBodyKinematics.cc"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/NuclearMass/NuclearMass.cc"
#include "TMath.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
void CalcEx() {
    const Double_t kInvalidValue = -999.0;
    const Double_t massBe = NuclearMass::Get("10Be").MassMeV;
    const Double_t massBeAmu = NuclearMass::Get("10Be").MassAMU;
    const Double_t massPAmu = NuclearMass::Get("1H").MassAMU;
    const Double_t beamEnergy = 1500;
    std::ifstream input("ExInput.txt");
    std::ofstream output("ExValues.csv");
    if (!input || !output) throw std::runtime_error("Cannot open excitation input/output");
    output << "SourceEntry,espriSide,ExOriginal_MeV,ExNN_MeV,OriginalOK,NNOK\n"
           << std::setprecision(17);
    Long64_t sourceEntry = 0;
    Int_t side = 0;
    Double_t theta = 0.0, originalEnergy = 0.0, nnEnergy = 0.0;
    while (input >> sourceEntry >> side >> theta >> originalEnergy >> nnEnergy) {
        const Double_t energies[2] = {originalEnergy, nnEnergy};
        Double_t excitation[2] = {kInvalidValue, kInvalidValue};
        Int_t valid[2] = {0, 0};
        for (Int_t method = 0; method < 2; ++method) {
            if (!std::isfinite(theta) || theta < 0.0 || theta > 180.0
                || !std::isfinite(energies[method]) || energies[method] <= 0.0
                || massBe + beamEnergy - energies[method] <= 0.0) continue;
            const Double_t value = TwoBodyKinematicsMissingMass(
                massBeAmu, massPAmu, massPAmu, massBeAmu, beamEnergy, energies[method],
                std::cos(theta * TMath::DegToRad()));
            if (std::isfinite(value)) {
                excitation[method] = value;
                valid[method] = 1;
            }
        }
        output << sourceEntry << ',' << side;
        for (Int_t method = 0; method < 2; ++method) output << ',' << excitation[method];
        for (Int_t method = 0; method < 2; ++method) output << ',' << valid[method];
        output << '\n';
    }
    output.close();
    if (!input.eof() || !output) throw std::runtime_error("Excitation data transfer failed");
}
