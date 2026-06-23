#include "TowBodyKinematics.h"

#include "constants.h"

#include <TGraph.h>
#include <TMultiGraph.h>
#include <TString.h>

#include <algorithm>
#include <cmath>

namespace {

constexpr Int_t kNTheta = 1200;
constexpr Double_t kThetaMinDeg = 0.1;
constexpr Double_t kThetaMaxDeg = 89.9;
constexpr Double_t kDegToRad = M_PI / 180.0;
constexpr Double_t kRadToDeg = 180.0 / M_PI;

enum class OutputMode {
    kProtonEnergyAngle,
    kResidueEnergyAngle,
    kAngleAngle
};

struct ReactionInput {
    Double_t beamEnergyPerU = 0.0;
    Double_t excitationEnergyD = 0.0;
    Double_t massA = 0.0;
    Double_t massB = 0.0;
    Double_t massC = 0.0;
    Double_t massD = 0.0;
    Double_t massDEffective = 0.0;
    Double_t massNumberA = 1.0;
    Double_t massNumberC = 1.0;
    Double_t massNumberD = 1.0;
    Double_t beamKineticEnergy = 0.0;
    Double_t beamMomentum = 0.0;
};

Double_t Square(Double_t value)
{
    return value * value;
}

Double_t MassNumberFromAmu(Double_t massAmu)
{
    const Double_t rounded = std::round(massAmu);
    return std::max(1.0, rounded);
}

ReactionInput MakeReaction(Double_t beamEnergyPerU,
                           Double_t excitationEnergyD,
                           Double_t massAAmu,
                           Double_t massBAmu,
                           Double_t massCAmu,
                           Double_t massDAmu)
{
    ReactionInput reaction;
    reaction.beamEnergyPerU = beamEnergyPerU;
    reaction.excitationEnergyD = excitationEnergyD;
    reaction.massNumberA = MassNumberFromAmu(massAAmu);
    reaction.massNumberC = MassNumberFromAmu(massCAmu);
    reaction.massNumberD = MassNumberFromAmu(massDAmu);

    reaction.massA = massAAmu * tof_constants::amu;
    reaction.massB = massBAmu * tof_constants::amu;
    reaction.massC = massCAmu * tof_constants::amu;
    reaction.massD = massDAmu * tof_constants::amu;
    reaction.massDEffective = reaction.massD + excitationEnergyD;

    reaction.beamKineticEnergy = beamEnergyPerU * reaction.massNumberA;
    reaction.beamMomentum = std::sqrt(std::max(
        0.0, Square(reaction.beamKineticEnergy) + 2.0 * reaction.massA * reaction.beamKineticEnergy));
    return reaction;
}

Bool_t SolveParticleCEnergy(const ReactionInput& reaction,
                            Double_t thetaCDeg,
                            Int_t branchSign,
                            Double_t& kineticEnergyC)
{
    const Double_t theta = thetaCDeg * kDegToRad;
    const Double_t cosTheta = std::cos(theta);
    const Double_t totalInitialEnergy = reaction.beamKineticEnergy + reaction.massA + reaction.massB;
    const Double_t beamMomentum2 = Square(reaction.beamMomentum);
    const Double_t delta =
        (Square(reaction.massA) + Square(reaction.massB) + Square(reaction.massC)
         - Square(reaction.massDEffective)
         + 2.0 * reaction.massB * (reaction.beamKineticEnergy + reaction.massA)) / 2.0;

    const Double_t denominator = Square(totalInitialEnergy) - beamMomentum2 * Square(cosTheta);
    if (denominator <= 0.0) {
        return kFALSE;
    }

    const Double_t radicand = Square(delta) - Square(reaction.massC) * denominator;
    if (radicand < 0.0) {
        return kFALSE;
    }

    const Double_t energyC =
        (totalInitialEnergy * delta
         + branchSign * reaction.beamMomentum * cosTheta * std::sqrt(radicand)) / denominator;

    kineticEnergyC = energyC - reaction.massC;
    return std::isfinite(kineticEnergyC) && kineticEnergyC >= 0.0;
}

Bool_t SolveResidueFromParticleC(const ReactionInput& reaction,
                                 Double_t thetaCDeg,
                                 Double_t kineticEnergyC,
                                 Double_t& thetaDDeg,
                                 Double_t& kineticEnergyD)
{
    const Double_t theta = thetaCDeg * kDegToRad;
    const Double_t momentumC = std::sqrt(std::max(
        0.0, Square(kineticEnergyC) + 2.0 * reaction.massC * kineticEnergyC));

    const Double_t pxD = reaction.beamMomentum - momentumC * std::cos(theta);
    const Double_t pyD = -momentumC * std::sin(theta);
    const Double_t momentumD = std::sqrt(Square(pxD) + Square(pyD));

    thetaDDeg = std::atan2(std::abs(pyD), pxD) * kRadToDeg;
    kineticEnergyD = std::sqrt(Square(momentumD) + Square(reaction.massDEffective))
                     - reaction.massDEffective;

    return std::isfinite(thetaDDeg) && std::isfinite(kineticEnergyD) && kineticEnergyD >= 0.0;
}

void AddBranchPoints(TGraph* graph,
                     const ReactionInput& reaction,
                     Int_t branchSign,
                     OutputMode mode)
{
    Int_t pointIndex = 0;
    for (Int_t i = 0; i < kNTheta; ++i) {
        const Double_t thetaC = kThetaMinDeg
                                + (kThetaMaxDeg - kThetaMinDeg) * i / (kNTheta - 1.0);

        Double_t kineticEnergyC = 0.0;
        if (!SolveParticleCEnergy(reaction, thetaC, branchSign, kineticEnergyC)) {
            continue;
        }

        Double_t thetaD = 0.0;
        Double_t kineticEnergyD = 0.0;
        if (!SolveResidueFromParticleC(reaction, thetaC, kineticEnergyC, thetaD, kineticEnergyD)) {
            continue;
        }

        if (mode == OutputMode::kProtonEnergyAngle) {
            graph->SetPoint(pointIndex++, thetaC, kineticEnergyC / reaction.massNumberC);
        } else if (mode == OutputMode::kResidueEnergyAngle) {
            graph->SetPoint(pointIndex++, thetaD, kineticEnergyD / reaction.massNumberD);
        } else {
            graph->SetPoint(pointIndex++, thetaD, thetaC);
        }
    }
}

TMultiGraph* BuildGraph(Double_t beamEnergyPerU,
                        Double_t excitationEnergyD,
                        Double_t massAAmu,
                        Double_t massBAmu,
                        Double_t massCAmu,
                        Double_t massDAmu,
                        OutputMode mode,
                        const char* name,
                        const char* title)
{
    const ReactionInput reaction = MakeReaction(
        beamEnergyPerU, excitationEnergyD, massAAmu, massBAmu, massCAmu, massDAmu);

    auto* multiGraph = new TMultiGraph(name, title);

    auto* plusBranch = new TGraph();
    plusBranch->SetName(TString::Format("%s_plus", name));
    AddBranchPoints(plusBranch, reaction, +1, mode);
    if (plusBranch->GetN() > 0) {
        plusBranch->SetLineStyle(kSolid);
        plusBranch->SetLineWidth(2);
        multiGraph->Add(plusBranch, "L");
    } else {
        delete plusBranch;
    }

    auto* minusBranch = new TGraph();
    minusBranch->SetName(TString::Format("%s_minus", name));
    AddBranchPoints(minusBranch, reaction, -1, mode);
    if (minusBranch->GetN() > 0) {
        minusBranch->SetLineStyle(kDashed);
        minusBranch->SetLineWidth(2);
        multiGraph->Add(minusBranch, "L");
    } else {
        delete minusBranch;
    }

    return multiGraph;
}

} // namespace

TMultiGraph* TowBodyKinematicsEnergyAngleProton(Double_t beamEnergyPerU,
                                                Double_t excitationEnergyD,
                                                Double_t massAAmu,
                                                Double_t massBAmu,
                                                Double_t massCAmu,
                                                Double_t massDAmu)
{
    return BuildGraph(beamEnergyPerU, excitationEnergyD, massAAmu, massBAmu, massCAmu, massDAmu,
                      OutputMode::kProtonEnergyAngle,
                      "mg_tow_body_proton_energy_angle",
                      ";#theta_{c}^{lab} (deg);K_{c}^{lab} (MeV/u)");
}

TMultiGraph* TowBodyKinematicsEnergyAngleResidue(Double_t beamEnergyPerU,
                                                 Double_t excitationEnergyD,
                                                 Double_t massAAmu,
                                                 Double_t massBAmu,
                                                 Double_t massCAmu,
                                                 Double_t massDAmu)
{
    return BuildGraph(beamEnergyPerU, excitationEnergyD, massAAmu, massBAmu, massCAmu, massDAmu,
                      OutputMode::kResidueEnergyAngle,
                      "mg_tow_body_residue_energy_angle",
                      ";#theta_{d}^{lab} (deg);K_{d}^{lab} (MeV/u)");
}

TMultiGraph* TowBodyKinematicsAngleAngle(Double_t beamEnergyPerU,
                                         Double_t excitationEnergyD,
                                         Double_t massAAmu,
                                         Double_t massBAmu,
                                         Double_t massCAmu,
                                         Double_t massDAmu)
{
    return BuildGraph(beamEnergyPerU, excitationEnergyD, massAAmu, massBAmu, massCAmu, massDAmu,
                      OutputMode::kAngleAngle,
                      "mg_tow_body_angle_angle",
                      ";#theta_{d}^{lab} (deg);#theta_{c}^{lab} (deg)");
}
