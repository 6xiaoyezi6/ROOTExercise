#ifndef TOW_BODY_KINEMATICS_H
#define TOW_BODY_KINEMATICS_H

#include <Rtypes.h>

class TMultiGraph;

// All mass inputs are ground-state masses in amu.
// beamEnergyPerU is the projectile kinetic energy in MeV/u.
// excitationEnergyD is added to particle d in MeV.
TMultiGraph* TowBodyKinematicsEnergyAngleProton(Double_t beamEnergyPerU,
                                                Double_t excitationEnergyD,
                                                Double_t massAAmu,
                                                Double_t massBAmu,
                                                Double_t massCAmu,
                                                Double_t massDAmu);

TMultiGraph* TowBodyKinematicsEnergyAngleResidue(Double_t beamEnergyPerU,
                                                 Double_t excitationEnergyD,
                                                 Double_t massAAmu,
                                                 Double_t massBAmu,
                                                 Double_t massCAmu,
                                                 Double_t massDAmu);

TMultiGraph* TowBodyKinematicsAngleAngle(Double_t beamEnergyPerU,
                                         Double_t excitationEnergyD,
                                         Double_t massAAmu,
                                         Double_t massBAmu,
                                         Double_t massCAmu,
                                         Double_t massDAmu);

#endif // TOW_BODY_KINEMATICS_H
