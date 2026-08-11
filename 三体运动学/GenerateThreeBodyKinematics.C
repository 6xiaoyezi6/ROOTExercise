// GenerateThreeBodyKinematics.C
//
// Pure relativistic three-body kinematics simulation:
//
//      A + T -> B + 1 + 2
//
// A: beam particle, moving along +z direction in lab
// T: target particle, at rest in lab
// B, 1, 2: final-state particles
//
// The only strict condition is four-momentum conservation:
//
//      P_A + P_T = P_B + P_1 + P_2
//
// No knockout dynamics.
// No internal momentum distribution.
// No cross section.
// No detector acceptance.
// No final-state interaction.
//
// Output:
//   1. TH2D histograms: T_lab vs theta_lab for B, 1, 2
//   2. Energy correlation histograms: T1 vs T2, TB vs T1, TB vs T2
//   3. Angle correlation histograms: thetaB vs theta1, thetaB vs theta2, theta1 vs theta2
//   4. Phi correlation histograms: phiB vs phi1, phiB vs phi2, phi1 vs phi2
//   5. Delta-phi distributions: dphiB1, dphiB2, dphi12
//   6. TTree containing event-by-event four-momentum information
//   7. ROOT file: output/PureThreeBodyKinematics.root
//
// Drawing is handled separately by Draw.C.
//
// Units:
//   Mass input: amu
//   Energy output: MeV
//   Momentum output: MeV/c
//   Angle output: degree

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TSystem.h"

using namespace std;

// ============================================================
// User interface
// ============================================================

void GenerateThreeBodyKinematics()
{
    gSystem->mkdir("output", kTRUE);

    // ------------------------------------------------------------
    // 1. User input
    // ------------------------------------------------------------
    //
    // Masses are given in amu.
    //
    // Example default:
    //
    //     10Be + p -> 6He + p + alpha
    //
    // If you simulate another reaction, only modify this block.
    //
    // 注意：
    // 如果你输入的是原子质量，并且反应前后总电子数相同，
    // 电子质量大部分会抵消。
    // 如果要做严格核反应运动学，建议输入核质量。
    //

    double MassA_u = 10.0135338;        // mass of A in amu, example: atomic mass of 10Be
    double MassT_u = 1.007276466621;    // mass of T in amu, proton mass
    double MassB_u = 6.01888589;        // mass of B in amu, example: atomic mass of 6He
    double Mass1_u = 1.007276466621;    // mass of particle 1 in amu, recoil proton
    double Mass2_u = 4.001506179127;    // mass of particle 2 in amu, alpha nuclear mass

    // Beam kinetic energy in MeV/u.
    double BeamEnergy_MeV_per_u = 150.0;

    // Conversion from MeV/u to total beam kinetic energy.
    //
    // For a beam of 10Be at 150 MeV/u, the total kinetic energy is usually:
    //
    //     T_A = 150 * 10 MeV
    //
    // Therefore, for 10Be one may use BeamA_ForEnergy = 10.0.
    //
    // If you want to use the mass in amu directly, set:
    //
    //     BeamA_ForEnergy = MassA_u;
    //
    double BeamA_ForEnergy = 10.0;

    // Number of Monte Carlo events.
    Long64_t NEvents = 10000000;

    // Random seed.
    // 0 means ROOT chooses a time-dependent seed.
    UInt_t RandomSeed = 0;

    // Histogram binning.
    int NThetaBins = 720;
    int NEnergyBins = 600;
    int NPhiBins   = 720;
    int NDphiBins  = 360;

    // Whether to fill histograms with simple phase-space density weight.
    //
    // false:
    //     all generated events have weight 1.
    //     Best for showing the allowed kinematic region.
    //
    // true:
    //     fill with a simple Lorentz-invariant phase-space factor.
    //     This changes density, not the boundary.
    //
    bool UsePhaseSpaceWeightInHist = false;

    // Output file name.
    const char* OutputFileName = "output/PureThreeBodyKinematics.root";

    // ------------------------------------------------------------
    // 2. Constants and mass conversion
    // ------------------------------------------------------------

    const double AMU_TO_MEV = 931.49410242; // 1 u = 931.49410242 MeV/c^2
    const double RadToDeg = 180.0 / TMath::Pi();

    double mA = MassA_u * AMU_TO_MEV;
    double mT = MassT_u * AMU_TO_MEV;
    double mB = MassB_u * AMU_TO_MEV;
    double m1 = Mass1_u * AMU_TO_MEV;
    double m2 = Mass2_u * AMU_TO_MEV;

    double TA_lab = BeamEnergy_MeV_per_u * BeamA_ForEnergy;
    double EA_lab = mA + TA_lab;
    double pA_lab = sqrt(EA_lab * EA_lab - mA * mA);

    TLorentzVector PA_lab(0.0, 0.0, pA_lab, EA_lab);
    TLorentzVector PT_lab(0.0, 0.0, 0.0, mT);

    TLorentzVector Ptot_lab = PA_lab + PT_lab;

    double s = Ptot_lab.M2();
    double W = sqrt(s);

    cout << "==================================================" << endl;
    cout << "Pure three-body kinematics simulation" << endl;
    cout << "Reaction: A + T -> B + 1 + 2" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Input masses [amu]:" << endl;
    cout << "  MassA = " << MassA_u << endl;
    cout << "  MassT = " << MassT_u << endl;
    cout << "  MassB = " << MassB_u << endl;
    cout << "  Mass1 = " << Mass1_u << endl;
    cout << "  Mass2 = " << Mass2_u << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Converted masses [MeV/c^2]:" << endl;
    cout << "  mA = " << mA << endl;
    cout << "  mT = " << mT << endl;
    cout << "  mB = " << mB << endl;
    cout << "  m1 = " << m1 << endl;
    cout << "  m2 = " << m2 << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Beam kinetic energy:" << endl;
    cout << "  BeamEnergy_MeV_per_u = " << BeamEnergy_MeV_per_u << " MeV/u" << endl;
    cout << "  BeamA_ForEnergy      = " << BeamA_ForEnergy << endl;
    cout << "  Total T_A_lab        = " << TA_lab << " MeV" << endl;
    cout << "  E_A_lab              = " << EA_lab << " MeV" << endl;
    cout << "  p_A_lab              = " << pA_lab << " MeV/c" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Total CM energy:" << endl;
    cout << "  sqrt(s) = W = " << W << " MeV" << endl;
    cout << "  threshold = mB + m1 + m2 = " << mB + m1 + m2 << " MeV" << endl;
    cout << "==================================================" << endl;

    if (W <= mB + m1 + m2) {
        cout << "[ERROR] Reaction is not kinematically allowed." << endl;
        cout << "        sqrt(s) <= mB + m1 + m2" << endl;
        return;
    }

    // Available final kinetic energy in lab.
    // This is used only to set histogram range.
    double TFinalAvailable_lab = Ptot_lab.E() - (mB + m1 + m2);
    double TAxisMax = 1.05 * TFinalAvailable_lab;
    if (TAxisMax < 10.0) TAxisMax = 10.0;

    // ------------------------------------------------------------
    // 3. Helper functions
    // ------------------------------------------------------------

    auto Kallen = [](double x, double y, double z) {
        return x*x + y*y + z*z - 2.0*x*y - 2.0*x*z - 2.0*y*z;
    };

    auto TwoBodyMomentum = [&](double M, double ma, double mb) {
        double lam = Kallen(M*M, ma*ma, mb*mb);

        if (lam < 0.0 && fabs(lam) < 1e-8) {
            lam = 0.0;
        }

        if (lam < 0.0) {
            cout << "[WARNING] Negative Kallen function: " << lam << endl;
            return 0.0;
        }

        return sqrt(lam) / (2.0 * M);
    };

    auto RandomDirection = [](TRandom3& rng, double p) {
        double costh = 2.0 * rng.Rndm() - 1.0;
        double sinth = sqrt(max(0.0, 1.0 - costh * costh));
        double phi = 2.0 * TMath::Pi() * rng.Rndm();

        double px = p * sinth * cos(phi);
        double py = p * sinth * sin(phi);
        double pz = p * costh;

        return TVector3(px, py, pz);
    };

    auto DeltaPhiDeg = [](double phiA_deg, double phiB_deg) {
        double dphi = phiA_deg - phiB_deg;

        while (dphi < 0.0)    dphi += 360.0;
        while (dphi >= 360.0) dphi -= 360.0;

        return dphi;
    };

    // ------------------------------------------------------------
    // 4. Histograms
    // ------------------------------------------------------------

    TH2D* hB_TTheta = new TH2D(
        "hB_TTheta",
        "Particle B allowed region;#theta_{B}^{lab} [deg];T_{B}^{lab} [MeV]",
        NThetaBins, 0.0, 180.0,
        NEnergyBins, 0.0, TAxisMax
    );

    TH2D* h1_TTheta = new TH2D(
        "h1_TTheta",
        "Particle 1 allowed region;#theta_{1}^{lab} [deg];T_{1}^{lab} [MeV]",
        NThetaBins, 0.0, 180.0,
        NEnergyBins, 0.0, TAxisMax
    );

    TH2D* h2_TTheta = new TH2D(
        "h2_TTheta",
        "Particle 2 allowed region;#theta_{2}^{lab} [deg];T_{2}^{lab} [MeV]",
        NThetaBins, 0.0, 180.0,
        NEnergyBins, 0.0, TAxisMax
    );

    // Energy-energy correlations.
    TH2D* hT1_T2 = new TH2D(
        "hT1_T2",
        "Energy correlation;T_{1}^{lab} [MeV];T_{2}^{lab} [MeV]",
        NEnergyBins, 0.0, TAxisMax,
        NEnergyBins, 0.0, TAxisMax
    );

    TH2D* hTB_T1 = new TH2D(
        "hTB_T1",
        "Energy correlation;T_{B}^{lab} [MeV];T_{1}^{lab} [MeV]",
        NEnergyBins, 0.0, TAxisMax,
        NEnergyBins, 0.0, TAxisMax
    );

    TH2D* hTB_T2 = new TH2D(
        "hTB_T2",
        "Energy correlation;T_{B}^{lab} [MeV];T_{2}^{lab} [MeV]",
        NEnergyBins, 0.0, TAxisMax,
        NEnergyBins, 0.0, TAxisMax
    );

    // Theta-theta correlations.
    TH2D* hThetaB_Theta1 = new TH2D(
        "hThetaB_Theta1",
        "Angle correlation;#theta_{B}^{lab} [deg];#theta_{1}^{lab} [deg]",
        NThetaBins, 0.0, 180.0,
        NThetaBins, 0.0, 180.0
    );

    TH2D* hThetaB_Theta2 = new TH2D(
        "hThetaB_Theta2",
        "Angle correlation;#theta_{B}^{lab} [deg];#theta_{2}^{lab} [deg]",
        NThetaBins, 0.0, 180.0,
        NThetaBins, 0.0, 180.0
    );

    TH2D* hTheta1_Theta2 = new TH2D(
        "hTheta1_Theta2",
        "Angle correlation;#theta_{1}^{lab} [deg];#theta_{2}^{lab} [deg]",
        NThetaBins, 0.0, 180.0,
        NThetaBins, 0.0, 180.0
    );

    // Phi-phi correlations.
    TH2D* hPhiB_Phi1 = new TH2D(
        "hPhiB_Phi1",
        "Phi correlation;#phi_{B}^{lab} [deg];#phi_{1}^{lab} [deg]",
        NPhiBins, -180.0, 180.0,
        NPhiBins, -180.0, 180.0
    );

    TH2D* hPhiB_Phi2 = new TH2D(
        "hPhiB_Phi2",
        "Phi correlation;#phi_{B}^{lab} [deg];#phi_{2}^{lab} [deg]",
        NPhiBins, -180.0, 180.0,
        NPhiBins, -180.0, 180.0
    );

    TH2D* hPhi1_Phi2 = new TH2D(
        "hPhi1_Phi2",
        "Phi correlation;#phi_{1}^{lab} [deg];#phi_{2}^{lab} [deg]",
        NPhiBins, -180.0, 180.0,
        NPhiBins, -180.0, 180.0
    );

    // Delta-phi distributions.
    TH1D* hDeltaPhiB1 = new TH1D(
        "hDeltaPhiB1",
        "#Delta#phi_{B1};#Delta#phi_{B1}^{lab} [deg];",
        NDphiBins, 0.0, 360.0
    );

    TH1D* hDeltaPhiB2 = new TH1D(
        "hDeltaPhiB2",
        "#Delta#phi_{B2};#Delta#phi_{B2}^{lab} [deg];",
        NDphiBins, 0.0, 360.0
    );

    TH1D* hDeltaPhi12 = new TH1D(
        "hDeltaPhi12",
        "#Delta#phi_{12};#Delta#phi_{12}^{lab} [deg];",
        NDphiBins, 0.0, 360.0
    );

    // ------------------------------------------------------------
    // 5. Envelopes: min/max kinetic energy at each theta bin
    // ------------------------------------------------------------

    vector<double> B_Tmin(NThetaBins,  numeric_limits<double>::max());
    vector<double> B_Tmax(NThetaBins, -numeric_limits<double>::max());

    vector<double> P1_Tmin(NThetaBins,  numeric_limits<double>::max());
    vector<double> P1_Tmax(NThetaBins, -numeric_limits<double>::max());

    vector<double> P2_Tmin(NThetaBins,  numeric_limits<double>::max());
    vector<double> P2_Tmax(NThetaBins, -numeric_limits<double>::max());

    auto FillEnvelope = [&](vector<double>& vmin,
                            vector<double>& vmax,
                            double theta_deg,
                            double T_lab)
    {
        if (theta_deg < 0.0 || theta_deg >= 180.0) return;

        int bin = int(theta_deg / 180.0 * NThetaBins);
        if (bin < 0 || bin >= NThetaBins) return;

        if (T_lab < vmin[bin]) vmin[bin] = T_lab;
        if (T_lab > vmax[bin]) vmax[bin] = T_lab;
    };

    auto MakeEnvelopeGraph = [&](const char* name,
                                 const vector<double>& v,
                                 bool isMin)
    {
        TGraph* g = new TGraph();
        g->SetName(name);

        int ip = 0;

        for (int i = 0; i < NThetaBins; ++i) {
            double val = v[i];

            bool valid = false;

            if (isMin) {
                valid = (val < numeric_limits<double>::max() / 10.0);
            } else {
                valid = (val > -numeric_limits<double>::max() / 10.0);
            }

            if (!valid) continue;

            double theta_center = (i + 0.5) * 180.0 / NThetaBins;
            g->SetPoint(ip, theta_center, val);
            ip++;
        }

        return g;
    };

    // ------------------------------------------------------------
    // 6. Output TTree
    // ------------------------------------------------------------

    TFile* fout = new TFile(OutputFileName, "RECREATE");

    TTree* tree = new TTree("tree", "Pure three-body kinematics");

    double TB_lab, T1_lab, T2_lab;
    double EB_lab, E1_lab, E2_lab;
    double pB_lab, p1_lab, p2_lab;
    double thetaB_lab, theta1_lab, theta2_lab;
    double phiB_lab, phi1_lab, phi2_lab;
    double dphiB1_lab, dphiB2_lab, dphi12_lab;

    double PBx_lab, PBy_lab, PBz_lab;
    double P1x_lab, P1y_lab, P1z_lab;
    double P2x_lab, P2y_lab, P2z_lab;

    double MX, MX2;
    double eventWeight;
    double dE_cons, dP_cons;

    tree->Branch("TB_lab", &TB_lab, "TB_lab/D");
    tree->Branch("T1_lab", &T1_lab, "T1_lab/D");
    tree->Branch("T2_lab", &T2_lab, "T2_lab/D");

    tree->Branch("EB_lab", &EB_lab, "EB_lab/D");
    tree->Branch("E1_lab", &E1_lab, "E1_lab/D");
    tree->Branch("E2_lab", &E2_lab, "E2_lab/D");

    tree->Branch("pB_lab", &pB_lab, "pB_lab/D");
    tree->Branch("p1_lab", &p1_lab, "p1_lab/D");
    tree->Branch("p2_lab", &p2_lab, "p2_lab/D");

    tree->Branch("thetaB_lab", &thetaB_lab, "thetaB_lab/D");
    tree->Branch("theta1_lab", &theta1_lab, "theta1_lab/D");
    tree->Branch("theta2_lab", &theta2_lab, "theta2_lab/D");

    tree->Branch("phiB_lab", &phiB_lab, "phiB_lab/D");
    tree->Branch("phi1_lab", &phi1_lab, "phi1_lab/D");
    tree->Branch("phi2_lab", &phi2_lab, "phi2_lab/D");

    tree->Branch("dphiB1_lab", &dphiB1_lab, "dphiB1_lab/D");
    tree->Branch("dphiB2_lab", &dphiB2_lab, "dphiB2_lab/D");
    tree->Branch("dphi12_lab", &dphi12_lab, "dphi12_lab/D");

    tree->Branch("PBx_lab", &PBx_lab, "PBx_lab/D");
    tree->Branch("PBy_lab", &PBy_lab, "PBy_lab/D");
    tree->Branch("PBz_lab", &PBz_lab, "PBz_lab/D");

    tree->Branch("P1x_lab", &P1x_lab, "P1x_lab/D");
    tree->Branch("P1y_lab", &P1y_lab, "P1y_lab/D");
    tree->Branch("P1z_lab", &P1z_lab, "P1z_lab/D");

    tree->Branch("P2x_lab", &P2x_lab, "P2x_lab/D");
    tree->Branch("P2y_lab", &P2y_lab, "P2y_lab/D");
    tree->Branch("P2z_lab", &P2z_lab, "P2z_lab/D");

    tree->Branch("MX", &MX, "MX/D");
    tree->Branch("MX2", &MX2, "MX2/D");
    tree->Branch("eventWeight", &eventWeight, "eventWeight/D");

    tree->Branch("dE_cons", &dE_cons, "dE_cons/D");
    tree->Branch("dP_cons", &dP_cons, "dP_cons/D");

    // ------------------------------------------------------------
    // 7. Three-body event generation
    // ------------------------------------------------------------

    TRandom3 rng(RandomSeed);

    TVector3 betaCM_to_lab = Ptot_lab.BoostVector();

    double MX_min = mB + m2;
    double MX_max = W - m1;

    double MX2_min = MX_min * MX_min;
    double MX2_max = MX_max * MX_max;

    double maxAbs_dE = 0.0;
    double max_dP = 0.0;

    for (Long64_t iev = 0; iev < NEvents; ++iev) {

        // --------------------------------------------------------
        // 7.1 Randomly choose the invariant mass of X = B + 2
        // --------------------------------------------------------

        MX2 = MX2_min + rng.Rndm() * (MX2_max - MX2_min);
        MX  = sqrt(MX2);

        // --------------------------------------------------------
        // 7.2 First two-body step in total CM:
        //
        //     Ptot -> 1 + X
        // --------------------------------------------------------

        double p1_cm_mag = TwoBodyMomentum(W, m1, MX);

        double E1_cm = sqrt(m1*m1 + p1_cm_mag*p1_cm_mag);
        double EX_cm = sqrt(MX*MX + p1_cm_mag*p1_cm_mag);

        TVector3 p1_cm_vec = RandomDirection(rng, p1_cm_mag);
        TVector3 pX_cm_vec = -p1_cm_vec;

        TLorentzVector P1_cm(p1_cm_vec, E1_cm);
        TLorentzVector PX_cm(pX_cm_vec, EX_cm);

        // --------------------------------------------------------
        // 7.3 Second two-body step in X rest frame:
        //
        //     X -> B + 2
        // --------------------------------------------------------

        double pB_X_mag = TwoBodyMomentum(MX, mB, m2);

        double EB_X = sqrt(mB*mB + pB_X_mag*pB_X_mag);
        double E2_X = sqrt(m2*m2 + pB_X_mag*pB_X_mag);

        TVector3 pB_X_vec = RandomDirection(rng, pB_X_mag);
        TVector3 p2_X_vec = -pB_X_vec;

        TLorentzVector PB_X(pB_X_vec, EB_X);
        TLorentzVector P2_X(p2_X_vec, E2_X);

        // Boost B and 2 from X rest frame to total CM.
        TVector3 betaX_to_cm = PX_cm.BoostVector();

        TLorentzVector PB_cm = PB_X;
        TLorentzVector P2_cm = P2_X;

        PB_cm.Boost(betaX_to_cm);
        P2_cm.Boost(betaX_to_cm);

        // --------------------------------------------------------
        // 7.4 Boost all final particles from total CM to lab
        // --------------------------------------------------------

        TLorentzVector PB_lab = PB_cm;
        TLorentzVector P1_lab_vec4 = P1_cm;
        TLorentzVector P2_lab_vec4 = P2_cm;

        PB_lab.Boost(betaCM_to_lab);
        P1_lab_vec4.Boost(betaCM_to_lab);
        P2_lab_vec4.Boost(betaCM_to_lab);

        // --------------------------------------------------------
        // 7.5 Check four-momentum conservation
        // --------------------------------------------------------

        TLorentzVector Psum_lab = PB_lab + P1_lab_vec4 + P2_lab_vec4;

        dE_cons = Ptot_lab.E() - Psum_lab.E();
        dP_cons = (Ptot_lab.Vect() - Psum_lab.Vect()).Mag();

        if (fabs(dE_cons) > maxAbs_dE) maxAbs_dE = fabs(dE_cons);
        if (dP_cons > max_dP) max_dP = dP_cons;

        // --------------------------------------------------------
        // 7.6 Extract lab kinetic energies and angles
        // --------------------------------------------------------

        EB_lab = PB_lab.E();
        E1_lab = P1_lab_vec4.E();
        E2_lab = P2_lab_vec4.E();

        TB_lab = EB_lab - mB;
        T1_lab = E1_lab - m1;
        T2_lab = E2_lab - m2;

        pB_lab = PB_lab.P();
        p1_lab = P1_lab_vec4.P();
        p2_lab = P2_lab_vec4.P();

        thetaB_lab = PB_lab.Vect().Theta() * RadToDeg;
        theta1_lab = P1_lab_vec4.Vect().Theta() * RadToDeg;
        theta2_lab = P2_lab_vec4.Vect().Theta() * RadToDeg;

        phiB_lab = PB_lab.Vect().Phi() * RadToDeg;
        phi1_lab = P1_lab_vec4.Vect().Phi() * RadToDeg;
        phi2_lab = P2_lab_vec4.Vect().Phi() * RadToDeg;

        dphiB1_lab = DeltaPhiDeg(phiB_lab, phi1_lab);
        dphiB2_lab = DeltaPhiDeg(phiB_lab, phi2_lab);
        dphi12_lab = DeltaPhiDeg(phi1_lab, phi2_lab);

        PBx_lab = PB_lab.Px();
        PBy_lab = PB_lab.Py();
        PBz_lab = PB_lab.Pz();

        P1x_lab = P1_lab_vec4.Px();
        P1y_lab = P1_lab_vec4.Py();
        P1z_lab = P1_lab_vec4.Pz();

        P2x_lab = P2_lab_vec4.Px();
        P2y_lab = P2_lab_vec4.Py();
        P2z_lab = P2_lab_vec4.Pz();

        // --------------------------------------------------------
        // 7.7 Optional phase-space density weight
        // --------------------------------------------------------
        //
        // Up to an irrelevant constant:
        //
        //     dPhi_3 ~ dM_X^2 dOmega_1 dOmega_B
        //              * p1_cm / W
        //              * pB_X  / M_X
        //
        // This affects only density, not the kinematic boundary.
        //

        eventWeight = (p1_cm_mag / W) * (pB_X_mag / MX);

        double fillWeight = 1.0;
        if (UsePhaseSpaceWeightInHist) fillWeight = eventWeight;

        // --------------------------------------------------------
        // 7.8 Fill histograms and tree
        // --------------------------------------------------------

        hB_TTheta->Fill(thetaB_lab, TB_lab, fillWeight);
        h1_TTheta->Fill(theta1_lab, T1_lab, fillWeight);
        h2_TTheta->Fill(theta2_lab, T2_lab, fillWeight);

        hT1_T2->Fill(T1_lab, T2_lab, fillWeight);
        hTB_T1->Fill(TB_lab, T1_lab, fillWeight);
        hTB_T2->Fill(TB_lab, T2_lab, fillWeight);

        hThetaB_Theta1->Fill(thetaB_lab, theta1_lab, fillWeight);
        hThetaB_Theta2->Fill(thetaB_lab, theta2_lab, fillWeight);
        hTheta1_Theta2->Fill(theta1_lab, theta2_lab, fillWeight);

        hPhiB_Phi1->Fill(phiB_lab, phi1_lab, fillWeight);
        hPhiB_Phi2->Fill(phiB_lab, phi2_lab, fillWeight);
        hPhi1_Phi2->Fill(phi1_lab, phi2_lab, fillWeight);

        hDeltaPhiB1->Fill(dphiB1_lab, fillWeight);
        hDeltaPhiB2->Fill(dphiB2_lab, fillWeight);
        hDeltaPhi12->Fill(dphi12_lab, fillWeight);

        FillEnvelope(B_Tmin,  B_Tmax,  thetaB_lab, TB_lab);
        FillEnvelope(P1_Tmin, P1_Tmax, theta1_lab, T1_lab);
        FillEnvelope(P2_Tmin, P2_Tmax, theta2_lab, T2_lab);

        tree->Fill();

        if ((iev + 1) % 200000 == 0) {
            cout << "Generated " << iev + 1 << " / " << NEvents << " events" << endl;
        }
    }

    cout << "==================================================" << endl;
    cout << "Generation finished." << endl;
    cout << "Max |energy conservation error| = " << maxAbs_dE << " MeV" << endl;
    cout << "Max |momentum conservation error| = " << max_dP << " MeV/c" << endl;
    cout << "==================================================" << endl;

    // ------------------------------------------------------------
    // 8. Build envelope graphs
    // ------------------------------------------------------------

    TGraph* gB_Tmin  = MakeEnvelopeGraph("gB_Tmin",  B_Tmin,  true);
    TGraph* gB_Tmax  = MakeEnvelopeGraph("gB_Tmax",  B_Tmax,  false);

    TGraph* g1_Tmin  = MakeEnvelopeGraph("g1_Tmin",  P1_Tmin, true);
    TGraph* g1_Tmax  = MakeEnvelopeGraph("g1_Tmax",  P1_Tmax, false);

    TGraph* g2_Tmin  = MakeEnvelopeGraph("g2_Tmin",  P2_Tmin, true);
    TGraph* g2_Tmax  = MakeEnvelopeGraph("g2_Tmax",  P2_Tmax, false);

    auto SetGraphStyle = [](TGraph* g, int color, int style) {
        if (!g) return;
        g->SetLineColor(color);
        g->SetLineWidth(2);
        g->SetLineStyle(style);
    };

    SetGraphStyle(gB_Tmin, kRed, 2);
    SetGraphStyle(gB_Tmax, kRed, 1);

    SetGraphStyle(g1_Tmin, kRed, 2);
    SetGraphStyle(g1_Tmax, kRed, 1);

    SetGraphStyle(g2_Tmin, kRed, 2);
    SetGraphStyle(g2_Tmax, kRed, 1);

    // ------------------------------------------------------------
    // 9. Write output ROOT file
    // ------------------------------------------------------------

    fout->cd();
    tree->Write();

    TDirectory* histogramDirectory = fout->mkdir("histograms");
    histogramDirectory->cd();

    hB_TTheta->Write();
    h1_TTheta->Write();
    h2_TTheta->Write();

    hT1_T2->Write();
    hTB_T1->Write();
    hTB_T2->Write();

    hThetaB_Theta1->Write();
    hThetaB_Theta2->Write();
    hTheta1_Theta2->Write();

    hPhiB_Phi1->Write();
    hPhiB_Phi2->Write();
    hPhi1_Phi2->Write();

    hDeltaPhiB1->Write();
    hDeltaPhiB2->Write();
    hDeltaPhi12->Write();

    gB_Tmin->Write();
    gB_Tmax->Write();

    g1_Tmin->Write();
    g1_Tmax->Write();

    g2_Tmin->Write();
    g2_Tmax->Write();

    fout->Close();

    cout << "Output ROOT file written to: " << OutputFileName << endl;
    cout << "Generated events: " << NEvents << endl;
    cout << "==================================================" << endl;
}
