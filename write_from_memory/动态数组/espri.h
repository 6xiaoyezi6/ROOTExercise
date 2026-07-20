//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jul 20 18:13:52 2026 by ROOT version 6.40.02
// from TTree tree/selected Event
// found on file: ppBe10.root_convert
//////////////////////////////////////////////////////////

#ifndef espri_h
#define espri_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"
#include "TVector3.h"
#include "TObject.h"

class espri {
public :
   TTree          *fChain;   ///<!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; ///<!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           RunNumber;
   Long64_t        EventNumber;
   Int_t           CoincidenceTrigger;
   Int_t           Trig_DSB;
   Int_t           Trig_BxESPRIxTEL;
   Int_t           Trig_BxESPRI;
   Int_t           Trig_BxTEL;
   Int_t           Trig_BxHODP;
   Int_t           Trig_BxHODF;
   Int_t           Trig_BxNEBULA;
   Int_t           F3TRaw;
   Int_t           F3QRaw;
   Double_t        F3TCal;
   Double_t        F3QCal;
   Double_t        F3T;
   Double_t        F3Q;
   Int_t           F7TRaw;
   Int_t           F7QRaw;
   Double_t        F7TCal;
   Double_t        F7QCal;
   Double_t        F7T;
   Double_t        F7Q;
   Int_t           SBT1TRaw;
   Int_t           SBT1QRaw;
   Double_t        SBT1TCal;
   Double_t        SBT1QCal;
   Double_t        SBT1T;
   Double_t        SBT1Q;
   Int_t           SBT2TRaw;
   Int_t           SBT2QRaw;
   Double_t        SBT2TCal;
   Double_t        SBT2QCal;
   Double_t        SBT2T;
   Double_t        SBT2Q;
   Double_t        F13T;
   Double_t        F13Q;
   Int_t           SBVTRaw;
   Int_t           SBVQRaw;
   Double_t        SBVTCal;
   Double_t        SBVQCal;
   Double_t        SBVT;
   Double_t        SBVQ;
   Int_t           BDC1_nhits;
   vector<double>  *BDC1_TDC;
   vector<double>  *BDC1_layerid;
   vector<double>  *BDC1_wireid;
   vector<int>     *BDC1_layer_hits;
   Int_t           BDC1_nlayerx;
   Int_t           BDC1_nlayery;
   vector<double>  *BDC1_Track_dl;
   vector<double>  *BDC1_Track_XY;
   vector<double>  *BDC1_Track_Z;
   vector<double>  *BDC1_Track_dXY;
   Double_t        BDC1_X;
   Double_t        BDC1_Y;
   Double_t        BDC1_ThetaX;
   Double_t        BDC1_ThetaY;
   Int_t           BDC2_nhits;
   vector<double>  *BDC2_TDC;
   vector<double>  *BDC2_layerid;
   vector<double>  *BDC2_wireid;
   vector<int>     *BDC2_layer_hits;
   Int_t           BDC2_nlayerx;
   Int_t           BDC2_nlayery;
   vector<double>  *BDC2_Track_dl;
   vector<double>  *BDC2_Track_XY;
   vector<double>  *BDC2_Track_Z;
   vector<double>  *BDC2_Track_dXY;
   Double_t        BDC2_X;
   Double_t        BDC2_Y;
   Double_t        BDC2_ThetaX;
   Double_t        BDC2_ThetaY;
   Int_t           FDC0_nhits;
   vector<double>  *FDC0_TDC;
   vector<double>  *FDC0_layerid;
   vector<double>  *FDC0_wireid;
   vector<int>     *FDC0_layer_hits;
   Int_t           FDC0_nlayerx;
   Int_t           FDC0_nlayery;
   vector<double>  *FDC0_Track_dl;
   vector<double>  *FDC0_Track_XY;
   vector<double>  *FDC0_Track_Z;
   vector<double>  *FDC0_Track_dXY;
   Double_t        FDC0_X;
   Double_t        FDC0_Y;
   Double_t        FDC0_ThetaX;
   Double_t        FDC0_ThetaY;
   Int_t           dssdQRaw[4][32];
   Int_t           dssdTRaw[2][32];
   Int_t           csiQRaw[7];
   Int_t           csiTRaw[7];
   Double_t        csiQPed[7];
   vector<int>     *plasTdcRaw0;
   vector<int>     *plasTdcRaw1;
   vector<int>     *plasTdcRaw2;
   vector<int>     *plasTdcRaw3;
   vector<int>     *rdcTdc;
   vector<int>     *rdcPlaneId;
   vector<int>     *rdcWireId;
   vector<int>     *rdcPlaneNHit;
   Int_t           naiQRaw[4][7];
   Int_t           plasQRaw[4];
   Int_t           plasTRaw[4];
   Int_t           plasTimeRef;
   Double_t        rdcX[2];
   Double_t        rdcY[2];
   Double_t        rdcA[2];
   Double_t        rdcB[2];
   Double_t        rdcChi2[2];
   Double_t        rdcDL[2][7];
   Double_t        rdcTch[2][7];
   Double_t        rdcRes[2][7];
   Double_t        naiQPed[4][7];
   Double_t        plasQPed[4];
   Int_t           rdcHit;
   Int_t           rdcHitSide[2];   //[rdcHit]
   Int_t           naiHit;
   Int_t           naiHitSide[9];   //[naiHit]
   Int_t           naiHitBarId[9];   //[naiHit]
   Int_t           plasHit;
   Int_t           plasHitSide[2];   //[plasHit]
   Int_t           plasTHit;
   Int_t           plasQHit;
   Int_t           espriHit;
   Int_t           espriSide;
   Double_t        espriRdcX;
   Double_t        espriRdcY;
   Double_t        espriPlasQPed;
   Int_t           espriNaiBarId;
   Double_t        espriNaiQPed;
   Int_t           hodQRaw[2][40];
   Double_t        hodQPed[2][40];
   Double_t        hodTCal[2][40];
   Double_t        hodQCal[2][40];
   Double_t        hodBarTCal[40];
   Double_t        TOF37;
   Double_t        Beta37;
   Double_t        Gamma37;
   Double_t        Ek37;
   Double_t        TOF713;
   Double_t        Beta713;
   Double_t        Gamma713;
   Double_t        Ek713;
   Double_t        beamFL;
 //TVector3        *vBeam;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *bdc1Position;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *bdc2Position;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *targetPosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *sbtPosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Double_t        fdc0Angle;
 //TVector3        *fdc0Position;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *vFDC0;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Int_t           hodNHit;
   Int_t           hodID[40];
   Int_t           hodTRaw[2][40];
   Double_t        hodBarQCal[40];
   Int_t           hodQHit;
   Int_t           hodQHitId[6];   //[hodQHit]
   Double_t        hodQHitQSyn[6];   //[hodQHit]
   Int_t           hodTHit;
   Int_t           hodTHitId[6];   //[hodTHit]
   Double_t        hodTHitTSyn[6];   //[hodTHit]
   Int_t           hodHit;
   Int_t           hodHitId[6];   //[hodHit]
   Double_t        hodHitT[6];   //[hodHit]
   Double_t        hodHitQ[6];   //[hodHit]
   Double_t        espriPlasE;
   Double_t        espriPlasT;
   Double_t        espriNaiE;
 //TVector3        *vESPRI;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *espriRdcPosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
 //TVector3        *espriPlasPosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Double_t        espriFL;
   Double_t        espriLocusAngle;
   Double_t        espriAngle;
 //TVector3        *espriPosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Int_t           dssdQRaw[4][32];
   Int_t           dssdTRaw[2][32];
   Double_t        dssdQPed[4][32];
   Int_t           csiQRaw[7];
   Int_t           csiTRaw[7];
   Double_t        csiQPed[7];
   Int_t           RefTime1;
   Int_t           RefTime2;
   Int_t           dssdTHit;
   Int_t           dssdTHitSide[34];   //[dssdTHit]
   Int_t           dssdTHitStripId[34];   //[dssdTHit]
   Double_t        dssdTHitTCal[34];   //[dssdTHit]
   Int_t           csiTHit;
   Int_t           csiTHitId[7];   //[csiTHit]
   Double_t        csiTHitTCal[7];   //[csiTHit]
   Int_t           teleHit;
   Int_t           teleHitSide[28];   //[teleHit]
   Int_t           teleHitFid[28];   //[teleHit]
   Int_t           teleHitBid[28];   //[teleHit]
   Double_t        teleHitDssdFQPed[28];   //[teleHit]
   Double_t        teleHitDssdBQPed[28];   //[teleHit]
   Double_t        teleHitDssdTCal[28];   //[teleHit]
   Int_t           teleHitCid[28];   //[teleHit]
   Double_t        teleHitCsiQPed[28];   //[teleHit]
   Double_t        teleHitCsiTCal[28];   //[teleHit]
   Double_t        teleHitDssdFE[28];   //[teleHit]
   Double_t        teleHitDssdBE[28];   //[teleHit]
   Double_t        teleHitCsiE[28];   //[teleHit]
   Int_t           bestHit;
   Int_t           teleSide;
   Int_t           teleCsiId;
   Int_t           teleDssdFid;
   Int_t           teleDssdBid;
   Double_t        teleCsiQPed;
   Double_t        teleDssdFQPed;
   Double_t        teleDssdBQPed;
   Double_t        teleCsiE;
   Double_t        teleDssdFE;
   Double_t        teleDssdBE;
   Double_t        teleDssdE;
   Double_t        teleDssdMaxE;
   Double_t        teleCsiT;
   Double_t        teleDssdT;
   Double_t        teleEnergy;
   Double_t        teleAngle;
   Double_t        teleLocusAngle;
 //TVector3        *telePosition;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Double_t        telePhi;
   Double_t        espriPhi;
   Double_t        fdc0Phi;
   Double_t        beamEnergy;
   Double_t        beamBeta;
   Double_t        tofSBTTarget;
 //TVector3        *vTele;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Double_t        fX;
   Double_t        fY;
   Double_t        fZ;
   Double_t        beamEnergy;
   Double_t        beamBeta;
   Double_t        tofSBTTarget;
   Double_t        ppPlasEPlas;
   Double_t        ppWinEPlas;
   Double_t        ppShtEPlas;
   Double_t        ppDegEPlas;
   Double_t        ppDegThPlas;
   Double_t        plasProtonEnergy;
   Double_t        ppPlasENai;
   Double_t        ppWinENai;
   Double_t        ppShtENai;
   Double_t        ppDegENai;
   Double_t        ppDegThNai;
   Double_t        naiEnergy;
   Double_t        naiProtonEnergy;
   Double_t        protonEnergy;
   Double_t        protonTheta;
   Double_t        protonPhi;
   Double_t        Ma;
   Double_t        Ea;
   Double_t        Pa;
   Double_t        Ta;
   Double_t        thetaa;
   Double_t        phia;
   Double_t        Mb;
   Double_t        Eb;
   Double_t        Pb;
   Double_t        Tb;
   Double_t        thetab;
   Double_t        phib;
   Double_t        MR0;
   Double_t        ER0;
   Double_t        PR0;
   Double_t        TR0;
   Double_t        thetaR0;
   Double_t        phiR0;
   Double_t        M1;
   Double_t        E1;
   Double_t        P1;
   Double_t        T1;
   Double_t        theta1;
   Double_t        phi1;
   Double_t        M2;
   Double_t        E2;
   Double_t        P2;
   Double_t        T2;
   Double_t        theta2;
   Double_t        phi2;
   Double_t        exEnergy;
   Double_t        phi12;

   // List of branches
   TBranch        *b_RunNumber;   ///<!
   TBranch        *b_EventNumber;   ///<!
   TBranch        *b_CoincidenceTrigger;   ///<!
   TBranch        *b_Trig_DSB;   ///<!
   TBranch        *b_Trig_BxESPRIxTEL;   ///<!
   TBranch        *b_Trig_BxESPRI;   ///<!
   TBranch        *b_Trig_BxTEL;   ///<!
   TBranch        *b_Trig_BxHODP;   ///<!
   TBranch        *b_Trig_BxHODF;   ///<!
   TBranch        *b_Trig_BxNEBULA;   ///<!
   TBranch        *b_F3TRaw;   ///<!
   TBranch        *b_F3QRaw;   ///<!
   TBranch        *b_F3TCal;   ///<!
   TBranch        *b_F3QCal;   ///<!
   TBranch        *b_F3T;   ///<!
   TBranch        *b_F3Q;   ///<!
   TBranch        *b_F7TRaw;   ///<!
   TBranch        *b_F7QRaw;   ///<!
   TBranch        *b_F7TCal;   ///<!
   TBranch        *b_F7QCal;   ///<!
   TBranch        *b_F7T;   ///<!
   TBranch        *b_F7Q;   ///<!
   TBranch        *b_SBT1TRaw;   ///<!
   TBranch        *b_SBT1QRaw;   ///<!
   TBranch        *b_SBT1TCal;   ///<!
   TBranch        *b_SBT1QCal;   ///<!
   TBranch        *b_SBT1T;   ///<!
   TBranch        *b_SBT1Q;   ///<!
   TBranch        *b_SBT2TRaw;   ///<!
   TBranch        *b_SBT2QRaw;   ///<!
   TBranch        *b_SBT2TCal;   ///<!
   TBranch        *b_SBT2QCal;   ///<!
   TBranch        *b_SBT2T;   ///<!
   TBranch        *b_SBT2Q;   ///<!
   TBranch        *b_F13T;   ///<!
   TBranch        *b_F13Q;   ///<!
   TBranch        *b_SBVTRaw;   ///<!
   TBranch        *b_SBVQRaw;   ///<!
   TBranch        *b_SBVTCal;   ///<!
   TBranch        *b_SBVQCal;   ///<!
   TBranch        *b_SBVT;   ///<!
   TBranch        *b_SBVQ;   ///<!
   TBranch        *b_BDC1_nhits;   ///<!
   TBranch        *b_BDC1_TDC;   ///<!
   TBranch        *b_BDC1_layerid;   ///<!
   TBranch        *b_BDC1_wireid;   ///<!
   TBranch        *b_BDC1_layer_hits;   ///<!
   TBranch        *b_BDC1_nlayerx;   ///<!
   TBranch        *b_BDC1_nlayery;   ///<!
   TBranch        *b_BDC1_Track_dl;   ///<!
   TBranch        *b_BDC1_Track_XY;   ///<!
   TBranch        *b_BDC1_Track_Z;   ///<!
   TBranch        *b_BDC1_Track_dXY;   ///<!
   TBranch        *b_BDC1_X;   ///<!
   TBranch        *b_BDC1_Y;   ///<!
   TBranch        *b_BDC1_ThetaX;   ///<!
   TBranch        *b_BDC1_ThetaY;   ///<!
   TBranch        *b_BDC2_nhits;   ///<!
   TBranch        *b_BDC2_TDC;   ///<!
   TBranch        *b_BDC2_layerid;   ///<!
   TBranch        *b_BDC2_wireid;   ///<!
   TBranch        *b_BDC2_layer_hits;   ///<!
   TBranch        *b_BDC2_nlayerx;   ///<!
   TBranch        *b_BDC2_nlayery;   ///<!
   TBranch        *b_BDC2_Track_dl;   ///<!
   TBranch        *b_BDC2_Track_XY;   ///<!
   TBranch        *b_BDC2_Track_Z;   ///<!
   TBranch        *b_BDC2_Track_dXY;   ///<!
   TBranch        *b_BDC2_X;   ///<!
   TBranch        *b_BDC2_Y;   ///<!
   TBranch        *b_BDC2_ThetaX;   ///<!
   TBranch        *b_BDC2_ThetaY;   ///<!
   TBranch        *b_FDC0_nhits;   ///<!
   TBranch        *b_FDC0_TDC;   ///<!
   TBranch        *b_FDC0_layerid;   ///<!
   TBranch        *b_FDC0_wireid;   ///<!
   TBranch        *b_FDC0_layer_hits;   ///<!
   TBranch        *b_FDC0_nlayerx;   ///<!
   TBranch        *b_FDC0_nlayery;   ///<!
   TBranch        *b_FDC0_Track_dl;   ///<!
   TBranch        *b_FDC0_Track_XY;   ///<!
   TBranch        *b_FDC0_Track_Z;   ///<!
   TBranch        *b_FDC0_Track_dXY;   ///<!
   TBranch        *b_FDC0_X;   ///<!
   TBranch        *b_FDC0_Y;   ///<!
   TBranch        *b_FDC0_ThetaX;   ///<!
   TBranch        *b_FDC0_ThetaY;   ///<!
   TBranch        *b_dssdQRaw;   ///<!
   TBranch        *b_dssdTRaw;   ///<!
   TBranch        *b_csiQRaw;   ///<!
   TBranch        *b_csiTRaw;   ///<!
   TBranch        *b_csiQPed;   ///<!
   TBranch        *b_plasTdcRaw0;   ///<!
   TBranch        *b_plasTdcRaw1;   ///<!
   TBranch        *b_plasTdcRaw2;   ///<!
   TBranch        *b_plasTdcRaw3;   ///<!
   TBranch        *b_rdcTdc;   ///<!
   TBranch        *b_rdcPlaneId;   ///<!
   TBranch        *b_rdcWireId;   ///<!
   TBranch        *b_rdcPlaneNHit;   ///<!
   TBranch        *b_naiQRaw;   ///<!
   TBranch        *b_plasQRaw;   ///<!
   TBranch        *b_plasTRaw;   ///<!
   TBranch        *b_plasTimeRef;   ///<!
   TBranch        *b_rdcX;   ///<!
   TBranch        *b_rdcY;   ///<!
   TBranch        *b_rdcA;   ///<!
   TBranch        *b_rdcB;   ///<!
   TBranch        *b_rdcChi2;   ///<!
   TBranch        *b_rdcDL;   ///<!
   TBranch        *b_rdcTch;   ///<!
   TBranch        *b_rdcRes;   ///<!
   TBranch        *b_naiQPed;   ///<!
   TBranch        *b_plasQPed;   ///<!
   TBranch        *b_rdcHit;   ///<!
   TBranch        *b_rdcHitSide;   ///<!
   TBranch        *b_naiHit;   ///<!
   TBranch        *b_naiHitSide;   ///<!
   TBranch        *b_naiHitBarId;   ///<!
   TBranch        *b_plasHit;   ///<!
   TBranch        *b_plasHitSide;   ///<!
   TBranch        *b_plasTHit;   ///<!
   TBranch        *b_plasQHit;   ///<!
   TBranch        *b_espriHit;   ///<!
   TBranch        *b_espriSide;   ///<!
   TBranch        *b_espriRdcX;   ///<!
   TBranch        *b_espriRdcY;   ///<!
   TBranch        *b_espriPlasQPed;   ///<!
   TBranch        *b_espriNaiBarId;   ///<!
   TBranch        *b_espriNaiQPed;   ///<!
   TBranch        *b_hodQRaw;   ///<!
   TBranch        *b_hodQPed;   ///<!
   TBranch        *b_hodTCal;   ///<!
   TBranch        *b_hodQCal;   ///<!
   TBranch        *b_hodBarTCal;   ///<!
   TBranch        *b_TOF37;   ///<!
   TBranch        *b_Beta37;   ///<!
   TBranch        *b_Gamma37;   ///<!
   TBranch        *b_Ek37;   ///<!
   TBranch        *b_TOF713;   ///<!
   TBranch        *b_Beta713;   ///<!
   TBranch        *b_Gamma713;   ///<!
   TBranch        *b_Ek713;   ///<!
   TBranch        *b_beamFL;   ///<!
   TBranch        *b_vBeam_fUniqueID;   ///<!
   TBranch        *b_vBeam_fBits;   ///<!
   TBranch        *b_vBeam_fX;   ///<!
   TBranch        *b_vBeam_fY;   ///<!
   TBranch        *b_vBeam_fZ;   ///<!
   TBranch        *b_bdc1Position_fUniqueID;   ///<!
   TBranch        *b_bdc1Position_fBits;   ///<!
   TBranch        *b_bdc1Position_fX;   ///<!
   TBranch        *b_bdc1Position_fY;   ///<!
   TBranch        *b_bdc1Position_fZ;   ///<!
   TBranch        *b_bdc2Position_fUniqueID;   ///<!
   TBranch        *b_bdc2Position_fBits;   ///<!
   TBranch        *b_bdc2Position_fX;   ///<!
   TBranch        *b_bdc2Position_fY;   ///<!
   TBranch        *b_bdc2Position_fZ;   ///<!
   TBranch        *b_targetPosition_fUniqueID;   ///<!
   TBranch        *b_targetPosition_fBits;   ///<!
   TBranch        *b_targetPosition_fX;   ///<!
   TBranch        *b_targetPosition_fY;   ///<!
   TBranch        *b_targetPosition_fZ;   ///<!
   TBranch        *b_sbtPosition_fUniqueID;   ///<!
   TBranch        *b_sbtPosition_fBits;   ///<!
   TBranch        *b_sbtPosition_fX;   ///<!
   TBranch        *b_sbtPosition_fY;   ///<!
   TBranch        *b_sbtPosition_fZ;   ///<!
   TBranch        *b_fdc0Angle;   ///<!
   TBranch        *b_fdc0Position_fUniqueID;   ///<!
   TBranch        *b_fdc0Position_fBits;   ///<!
   TBranch        *b_fdc0Position_fX;   ///<!
   TBranch        *b_fdc0Position_fY;   ///<!
   TBranch        *b_fdc0Position_fZ;   ///<!
   TBranch        *b_vFDC0_fUniqueID;   ///<!
   TBranch        *b_vFDC0_fBits;   ///<!
   TBranch        *b_vFDC0_fX;   ///<!
   TBranch        *b_vFDC0_fY;   ///<!
   TBranch        *b_vFDC0_fZ;   ///<!
   TBranch        *b_hodNHit;   ///<!
   TBranch        *b_hodID;   ///<!
   TBranch        *b_hodTRaw;   ///<!
   TBranch        *b_hodBarQCal;   ///<!
   TBranch        *b_hodQHit;   ///<!
   TBranch        *b_hodQHitId;   ///<!
   TBranch        *b_hodQHitQSyn;   ///<!
   TBranch        *b_hodTHit;   ///<!
   TBranch        *b_hodTHitId;   ///<!
   TBranch        *b_hodTHitTSyn;   ///<!
   TBranch        *b_hodHit;   ///<!
   TBranch        *b_hodHitId;   ///<!
   TBranch        *b_hodHitT;   ///<!
   TBranch        *b_hodHitQ;   ///<!
   TBranch        *b_espriPlasE;   ///<!
   TBranch        *b_espriPlasT;   ///<!
   TBranch        *b_espriNaiE;   ///<!
   TBranch        *b_vESPRI_fUniqueID;   ///<!
   TBranch        *b_vESPRI_fBits;   ///<!
   TBranch        *b_vESPRI_fX;   ///<!
   TBranch        *b_vESPRI_fY;   ///<!
   TBranch        *b_vESPRI_fZ;   ///<!
   TBranch        *b_espriRdcPosition_fUniqueID;   ///<!
   TBranch        *b_espriRdcPosition_fBits;   ///<!
   TBranch        *b_espriRdcPosition_fX;   ///<!
   TBranch        *b_espriRdcPosition_fY;   ///<!
   TBranch        *b_espriRdcPosition_fZ;   ///<!
   TBranch        *b_espriPlasPosition_fUniqueID;   ///<!
   TBranch        *b_espriPlasPosition_fBits;   ///<!
   TBranch        *b_espriPlasPosition_fX;   ///<!
   TBranch        *b_espriPlasPosition_fY;   ///<!
   TBranch        *b_espriPlasPosition_fZ;   ///<!
   TBranch        *b_espriFL;   ///<!
   TBranch        *b_espriLocusAngle;   ///<!
   TBranch        *b_espriAngle;   ///<!
   TBranch        *b_espriPosition_fUniqueID;   ///<!
   TBranch        *b_espriPosition_fBits;   ///<!
   TBranch        *b_espriPosition_fX;   ///<!
   TBranch        *b_espriPosition_fY;   ///<!
   TBranch        *b_espriPosition_fZ;   ///<!
   TBranch        *b_dssdQRaw;   ///<!
   TBranch        *b_dssdTRaw;   ///<!
   TBranch        *b_dssdQPed;   ///<!
   TBranch        *b_csiQRaw;   ///<!
   TBranch        *b_csiTRaw;   ///<!
   TBranch        *b_csiQPed;   ///<!
   TBranch        *b_RefTime1;   ///<!
   TBranch        *b_RefTime2;   ///<!
   TBranch        *b_dssdTHit;   ///<!
   TBranch        *b_dssdTHitSide;   ///<!
   TBranch        *b_dssdTHitStripId;   ///<!
   TBranch        *b_dssdTHitTCal;   ///<!
   TBranch        *b_csiTHit;   ///<!
   TBranch        *b_csiTHitId;   ///<!
   TBranch        *b_csiTHitTCal;   ///<!
   TBranch        *b_teleHit;   ///<!
   TBranch        *b_teleHitSide;   ///<!
   TBranch        *b_teleHitFid;   ///<!
   TBranch        *b_teleHitBid;   ///<!
   TBranch        *b_teleHitDssdFQPed;   ///<!
   TBranch        *b_teleHitDssdBQPed;   ///<!
   TBranch        *b_teleHitDssdTCal;   ///<!
   TBranch        *b_teleHitCid;   ///<!
   TBranch        *b_teleHitCsiQPed;   ///<!
   TBranch        *b_teleHitCsiTCal;   ///<!
   TBranch        *b_teleHitDssdFE;   ///<!
   TBranch        *b_teleHitDssdBE;   ///<!
   TBranch        *b_teleHitCsiE;   ///<!
   TBranch        *b_bestHit;   ///<!
   TBranch        *b_teleSide;   ///<!
   TBranch        *b_teleCsiId;   ///<!
   TBranch        *b_teleDssdFid;   ///<!
   TBranch        *b_teleDssdBid;   ///<!
   TBranch        *b_teleCsiQPed;   ///<!
   TBranch        *b_teleDssdFQPed;   ///<!
   TBranch        *b_teleDssdBQPed;   ///<!
   TBranch        *b_teleCsiE;   ///<!
   TBranch        *b_teleDssdFE;   ///<!
   TBranch        *b_teleDssdBE;   ///<!
   TBranch        *b_teleDssdE;   ///<!
   TBranch        *b_teleDssdMaxE;   ///<!
   TBranch        *b_teleCsiT;   ///<!
   TBranch        *b_teleDssdT;   ///<!
   TBranch        *b_teleEnergy;   ///<!
   TBranch        *b_teleAngle;   ///<!
   TBranch        *b_teleLocusAngle;   ///<!
   TBranch        *b_telePosition_fUniqueID;   ///<!
   TBranch        *b_telePosition_fBits;   ///<!
   TBranch        *b_telePosition_fX;   ///<!
   TBranch        *b_telePosition_fY;   ///<!
   TBranch        *b_telePosition_fZ;   ///<!
   TBranch        *b_telePhi;   ///<!
   TBranch        *b_espriPhi;   ///<!
   TBranch        *b_fdc0Phi;   ///<!
   TBranch        *b_beamEnergy;   ///<!
   TBranch        *b_beamBeta;   ///<!
   TBranch        *b_tofSBTTarget;   ///<!
   TBranch        *b_vTele_fUniqueID;   ///<!
   TBranch        *b_vTele_fBits;   ///<!
   TBranch        *b_vTele_fX;   ///<!
   TBranch        *b_vTele_fY;   ///<!
   TBranch        *b_vTele_fZ;   ///<!
   TBranch        *b_beamEnergy;   ///<!
   TBranch        *b_beamBeta;   ///<!
   TBranch        *b_tofSBTTarget;   ///<!
   TBranch        *b_ppPlasEPlas;   ///<!
   TBranch        *b_ppWinEPlas;   ///<!
   TBranch        *b_ppShtEPlas;   ///<!
   TBranch        *b_ppDegEPlas;   ///<!
   TBranch        *b_ppDegThPlas;   ///<!
   TBranch        *b_plasProtonEnergy;   ///<!
   TBranch        *b_ppPlasENai;   ///<!
   TBranch        *b_ppWinENai;   ///<!
   TBranch        *b_ppShtENai;   ///<!
   TBranch        *b_ppDegENai;   ///<!
   TBranch        *b_ppDegThNai;   ///<!
   TBranch        *b_naiEnergy;   ///<!
   TBranch        *b_naiProtonEnergy;   ///<!
   TBranch        *b_protonEnergy;   ///<!
   TBranch        *b_protonTheta;   ///<!
   TBranch        *b_protonPhi;   ///<!
   TBranch        *b_Ma;   ///<!
   TBranch        *b_Ea;   ///<!
   TBranch        *b_Pa;   ///<!
   TBranch        *b_Ta;   ///<!
   TBranch        *b_thetaa;   ///<!
   TBranch        *b_phia;   ///<!
   TBranch        *b_Mb;   ///<!
   TBranch        *b_Eb;   ///<!
   TBranch        *b_Pb;   ///<!
   TBranch        *b_Tb;   ///<!
   TBranch        *b_thetab;   ///<!
   TBranch        *b_phib;   ///<!
   TBranch        *b_MR0;   ///<!
   TBranch        *b_ER0;   ///<!
   TBranch        *b_PR0;   ///<!
   TBranch        *b_TR0;   ///<!
   TBranch        *b_thetaR0;   ///<!
   TBranch        *b_phiR0;   ///<!
   TBranch        *b_M1;   ///<!
   TBranch        *b_E1;   ///<!
   TBranch        *b_P1;   ///<!
   TBranch        *b_T1;   ///<!
   TBranch        *b_theta1;   ///<!
   TBranch        *b_phi1;   ///<!
   TBranch        *b_M2;   ///<!
   TBranch        *b_E2;   ///<!
   TBranch        *b_P2;   ///<!
   TBranch        *b_T2;   ///<!
   TBranch        *b_theta2;   ///<!
   TBranch        *b_phi2;   ///<!
   TBranch        *b_exEnergy;   ///<!
   TBranch        *b_phi12;   ///<!

   espri(TTree *tree=0);
   virtual ~espri();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef espri_cxx
espri::espri(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("ppBe10.root_convert");
      if (!f || !f->IsOpen()) {
         f = new TFile("ppBe10.root_convert");
      }
      f->GetObject("tree",tree);

   }
   Init(tree);
}

espri::~espri()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t espri::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t espri::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void espri::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.

   // Set object pointer
   BDC1_TDC = 0;
   BDC1_layerid = 0;
   BDC1_wireid = 0;
   BDC1_layer_hits = 0;
   BDC1_Track_dl = 0;
   BDC1_Track_XY = 0;
   BDC1_Track_Z = 0;
   BDC1_Track_dXY = 0;
   BDC2_TDC = 0;
   BDC2_layerid = 0;
   BDC2_wireid = 0;
   BDC2_layer_hits = 0;
   BDC2_Track_dl = 0;
   BDC2_Track_XY = 0;
   BDC2_Track_Z = 0;
   BDC2_Track_dXY = 0;
   FDC0_TDC = 0;
   FDC0_layerid = 0;
   FDC0_wireid = 0;
   FDC0_layer_hits = 0;
   FDC0_Track_dl = 0;
   FDC0_Track_XY = 0;
   FDC0_Track_Z = 0;
   FDC0_Track_dXY = 0;
   plasTdcRaw0 = 0;
   plasTdcRaw1 = 0;
   plasTdcRaw2 = 0;
   plasTdcRaw3 = 0;
   rdcTdc = 0;
   rdcPlaneId = 0;
   rdcWireId = 0;
   rdcPlaneNHit = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("RunNumber", &RunNumber, &b_RunNumber);
   fChain->SetBranchAddress("EventNumber", &EventNumber, &b_EventNumber);
   fChain->SetBranchAddress("CoincidenceTrigger", &CoincidenceTrigger, &b_CoincidenceTrigger);
   fChain->SetBranchAddress("Trig_DSB", &Trig_DSB, &b_Trig_DSB);
   fChain->SetBranchAddress("Trig_BxESPRIxTEL", &Trig_BxESPRIxTEL, &b_Trig_BxESPRIxTEL);
   fChain->SetBranchAddress("Trig_BxESPRI", &Trig_BxESPRI, &b_Trig_BxESPRI);
   fChain->SetBranchAddress("Trig_BxTEL", &Trig_BxTEL, &b_Trig_BxTEL);
   fChain->SetBranchAddress("Trig_BxHODP", &Trig_BxHODP, &b_Trig_BxHODP);
   fChain->SetBranchAddress("Trig_BxHODF", &Trig_BxHODF, &b_Trig_BxHODF);
   fChain->SetBranchAddress("Trig_BxNEBULA", &Trig_BxNEBULA, &b_Trig_BxNEBULA);
   fChain->SetBranchAddress("F3TRaw", &F3TRaw, &b_F3TRaw);
   fChain->SetBranchAddress("F3QRaw", &F3QRaw, &b_F3QRaw);
   fChain->SetBranchAddress("F3TCal", &F3TCal, &b_F3TCal);
   fChain->SetBranchAddress("F3QCal", &F3QCal, &b_F3QCal);
   fChain->SetBranchAddress("F3T", &F3T, &b_F3T);
   fChain->SetBranchAddress("F3Q", &F3Q, &b_F3Q);
   fChain->SetBranchAddress("F7TRaw", &F7TRaw, &b_F7TRaw);
   fChain->SetBranchAddress("F7QRaw", &F7QRaw, &b_F7QRaw);
   fChain->SetBranchAddress("F7TCal", &F7TCal, &b_F7TCal);
   fChain->SetBranchAddress("F7QCal", &F7QCal, &b_F7QCal);
   fChain->SetBranchAddress("F7T", &F7T, &b_F7T);
   fChain->SetBranchAddress("F7Q", &F7Q, &b_F7Q);
   fChain->SetBranchAddress("SBT1TRaw", &SBT1TRaw, &b_SBT1TRaw);
   fChain->SetBranchAddress("SBT1QRaw", &SBT1QRaw, &b_SBT1QRaw);
   fChain->SetBranchAddress("SBT1TCal", &SBT1TCal, &b_SBT1TCal);
   fChain->SetBranchAddress("SBT1QCal", &SBT1QCal, &b_SBT1QCal);
   fChain->SetBranchAddress("SBT1T", &SBT1T, &b_SBT1T);
   fChain->SetBranchAddress("SBT1Q", &SBT1Q, &b_SBT1Q);
   fChain->SetBranchAddress("SBT2TRaw", &SBT2TRaw, &b_SBT2TRaw);
   fChain->SetBranchAddress("SBT2QRaw", &SBT2QRaw, &b_SBT2QRaw);
   fChain->SetBranchAddress("SBT2TCal", &SBT2TCal, &b_SBT2TCal);
   fChain->SetBranchAddress("SBT2QCal", &SBT2QCal, &b_SBT2QCal);
   fChain->SetBranchAddress("SBT2T", &SBT2T, &b_SBT2T);
   fChain->SetBranchAddress("SBT2Q", &SBT2Q, &b_SBT2Q);
   fChain->SetBranchAddress("F13T", &F13T, &b_F13T);
   fChain->SetBranchAddress("F13Q", &F13Q, &b_F13Q);
   fChain->SetBranchAddress("SBVTRaw", &SBVTRaw, &b_SBVTRaw);
   fChain->SetBranchAddress("SBVQRaw", &SBVQRaw, &b_SBVQRaw);
   fChain->SetBranchAddress("SBVTCal", &SBVTCal, &b_SBVTCal);
   fChain->SetBranchAddress("SBVQCal", &SBVQCal, &b_SBVQCal);
   fChain->SetBranchAddress("SBVT", &SBVT, &b_SBVT);
   fChain->SetBranchAddress("SBVQ", &SBVQ, &b_SBVQ);
   fChain->SetBranchAddress("BDC1_nhits", &BDC1_nhits, &b_BDC1_nhits);
   fChain->SetBranchAddress("BDC1_TDC", &BDC1_TDC, &b_BDC1_TDC);
   fChain->SetBranchAddress("BDC1_layerid", &BDC1_layerid, &b_BDC1_layerid);
   fChain->SetBranchAddress("BDC1_wireid", &BDC1_wireid, &b_BDC1_wireid);
   fChain->SetBranchAddress("BDC1_layer_hits", &BDC1_layer_hits, &b_BDC1_layer_hits);
   fChain->SetBranchAddress("BDC1_nlayerx", &BDC1_nlayerx, &b_BDC1_nlayerx);
   fChain->SetBranchAddress("BDC1_nlayery", &BDC1_nlayery, &b_BDC1_nlayery);
   fChain->SetBranchAddress("BDC1_Track_dl", &BDC1_Track_dl, &b_BDC1_Track_dl);
   fChain->SetBranchAddress("BDC1_Track_XY", &BDC1_Track_XY, &b_BDC1_Track_XY);
   fChain->SetBranchAddress("BDC1_Track_Z", &BDC1_Track_Z, &b_BDC1_Track_Z);
   fChain->SetBranchAddress("BDC1_Track_dXY", &BDC1_Track_dXY, &b_BDC1_Track_dXY);
   fChain->SetBranchAddress("BDC1_X", &BDC1_X, &b_BDC1_X);
   fChain->SetBranchAddress("BDC1_Y", &BDC1_Y, &b_BDC1_Y);
   fChain->SetBranchAddress("BDC1_ThetaX", &BDC1_ThetaX, &b_BDC1_ThetaX);
   fChain->SetBranchAddress("BDC1_ThetaY", &BDC1_ThetaY, &b_BDC1_ThetaY);
   fChain->SetBranchAddress("BDC2_nhits", &BDC2_nhits, &b_BDC2_nhits);
   fChain->SetBranchAddress("BDC2_TDC", &BDC2_TDC, &b_BDC2_TDC);
   fChain->SetBranchAddress("BDC2_layerid", &BDC2_layerid, &b_BDC2_layerid);
   fChain->SetBranchAddress("BDC2_wireid", &BDC2_wireid, &b_BDC2_wireid);
   fChain->SetBranchAddress("BDC2_layer_hits", &BDC2_layer_hits, &b_BDC2_layer_hits);
   fChain->SetBranchAddress("BDC2_nlayerx", &BDC2_nlayerx, &b_BDC2_nlayerx);
   fChain->SetBranchAddress("BDC2_nlayery", &BDC2_nlayery, &b_BDC2_nlayery);
   fChain->SetBranchAddress("BDC2_Track_dl", &BDC2_Track_dl, &b_BDC2_Track_dl);
   fChain->SetBranchAddress("BDC2_Track_XY", &BDC2_Track_XY, &b_BDC2_Track_XY);
   fChain->SetBranchAddress("BDC2_Track_Z", &BDC2_Track_Z, &b_BDC2_Track_Z);
   fChain->SetBranchAddress("BDC2_Track_dXY", &BDC2_Track_dXY, &b_BDC2_Track_dXY);
   fChain->SetBranchAddress("BDC2_X", &BDC2_X, &b_BDC2_X);
   fChain->SetBranchAddress("BDC2_Y", &BDC2_Y, &b_BDC2_Y);
   fChain->SetBranchAddress("BDC2_ThetaX", &BDC2_ThetaX, &b_BDC2_ThetaX);
   fChain->SetBranchAddress("BDC2_ThetaY", &BDC2_ThetaY, &b_BDC2_ThetaY);
   fChain->SetBranchAddress("FDC0_nhits", &FDC0_nhits, &b_FDC0_nhits);
   fChain->SetBranchAddress("FDC0_TDC", &FDC0_TDC, &b_FDC0_TDC);
   fChain->SetBranchAddress("FDC0_layerid", &FDC0_layerid, &b_FDC0_layerid);
   fChain->SetBranchAddress("FDC0_wireid", &FDC0_wireid, &b_FDC0_wireid);
   fChain->SetBranchAddress("FDC0_layer_hits", &FDC0_layer_hits, &b_FDC0_layer_hits);
   fChain->SetBranchAddress("FDC0_nlayerx", &FDC0_nlayerx, &b_FDC0_nlayerx);
   fChain->SetBranchAddress("FDC0_nlayery", &FDC0_nlayery, &b_FDC0_nlayery);
   fChain->SetBranchAddress("FDC0_Track_dl", &FDC0_Track_dl, &b_FDC0_Track_dl);
   fChain->SetBranchAddress("FDC0_Track_XY", &FDC0_Track_XY, &b_FDC0_Track_XY);
   fChain->SetBranchAddress("FDC0_Track_Z", &FDC0_Track_Z, &b_FDC0_Track_Z);
   fChain->SetBranchAddress("FDC0_Track_dXY", &FDC0_Track_dXY, &b_FDC0_Track_dXY);
   fChain->SetBranchAddress("FDC0_X", &FDC0_X, &b_FDC0_X);
   fChain->SetBranchAddress("FDC0_Y", &FDC0_Y, &b_FDC0_Y);
   fChain->SetBranchAddress("FDC0_ThetaX", &FDC0_ThetaX, &b_FDC0_ThetaX);
   fChain->SetBranchAddress("FDC0_ThetaY", &FDC0_ThetaY, &b_FDC0_ThetaY);
   fChain->SetBranchAddress("dssdQRaw", dssdQRaw, &b_dssdQRaw);
   fChain->SetBranchAddress("dssdTRaw", dssdTRaw, &b_dssdTRaw);
   fChain->SetBranchAddress("csiQRaw", csiQRaw, &b_csiQRaw);
   fChain->SetBranchAddress("csiTRaw", csiTRaw, &b_csiTRaw);
   fChain->SetBranchAddress("csiQPed", csiQPed, &b_csiQPed);
   fChain->SetBranchAddress("plasTdcRaw0", &plasTdcRaw0, &b_plasTdcRaw0);
   fChain->SetBranchAddress("plasTdcRaw1", &plasTdcRaw1, &b_plasTdcRaw1);
   fChain->SetBranchAddress("plasTdcRaw2", &plasTdcRaw2, &b_plasTdcRaw2);
   fChain->SetBranchAddress("plasTdcRaw3", &plasTdcRaw3, &b_plasTdcRaw3);
   fChain->SetBranchAddress("rdcTdc", &rdcTdc, &b_rdcTdc);
   fChain->SetBranchAddress("rdcPlaneId", &rdcPlaneId, &b_rdcPlaneId);
   fChain->SetBranchAddress("rdcWireId", &rdcWireId, &b_rdcWireId);
   fChain->SetBranchAddress("rdcPlaneNHit", &rdcPlaneNHit, &b_rdcPlaneNHit);
   fChain->SetBranchAddress("naiQRaw", naiQRaw, &b_naiQRaw);
   fChain->SetBranchAddress("plasQRaw", plasQRaw, &b_plasQRaw);
   fChain->SetBranchAddress("plasTRaw", plasTRaw, &b_plasTRaw);
   fChain->SetBranchAddress("plasTimeRef", &plasTimeRef, &b_plasTimeRef);
   fChain->SetBranchAddress("rdcX", rdcX, &b_rdcX);
   fChain->SetBranchAddress("rdcY", rdcY, &b_rdcY);
   fChain->SetBranchAddress("rdcA", rdcA, &b_rdcA);
   fChain->SetBranchAddress("rdcB", rdcB, &b_rdcB);
   fChain->SetBranchAddress("rdcChi2", rdcChi2, &b_rdcChi2);
   fChain->SetBranchAddress("rdcDL", rdcDL, &b_rdcDL);
   fChain->SetBranchAddress("rdcTch", rdcTch, &b_rdcTch);
   fChain->SetBranchAddress("rdcRes", rdcRes, &b_rdcRes);
   fChain->SetBranchAddress("naiQPed", naiQPed, &b_naiQPed);
   fChain->SetBranchAddress("plasQPed", plasQPed, &b_plasQPed);
   fChain->SetBranchAddress("rdcHit", &rdcHit, &b_rdcHit);
   fChain->SetBranchAddress("rdcHitSide", rdcHitSide, &b_rdcHitSide);
   fChain->SetBranchAddress("naiHit", &naiHit, &b_naiHit);
   fChain->SetBranchAddress("naiHitSide", naiHitSide, &b_naiHitSide);
   fChain->SetBranchAddress("naiHitBarId", naiHitBarId, &b_naiHitBarId);
   fChain->SetBranchAddress("plasHit", &plasHit, &b_plasHit);
   fChain->SetBranchAddress("plasHitSide", plasHitSide, &b_plasHitSide);
   fChain->SetBranchAddress("plasTHit", &plasTHit, &b_plasTHit);
   fChain->SetBranchAddress("plasQHit", &plasQHit, &b_plasQHit);
   fChain->SetBranchAddress("espriHit", &espriHit, &b_espriHit);
   fChain->SetBranchAddress("espriSide", &espriSide, &b_espriSide);
   fChain->SetBranchAddress("espriRdcX", &espriRdcX, &b_espriRdcX);
   fChain->SetBranchAddress("espriRdcY", &espriRdcY, &b_espriRdcY);
   fChain->SetBranchAddress("espriPlasQPed", &espriPlasQPed, &b_espriPlasQPed);
   fChain->SetBranchAddress("espriNaiBarId", &espriNaiBarId, &b_espriNaiBarId);
   fChain->SetBranchAddress("espriNaiQPed", &espriNaiQPed, &b_espriNaiQPed);
   fChain->SetBranchAddress("hodQRaw", hodQRaw, &b_hodQRaw);
   fChain->SetBranchAddress("hodQPed", hodQPed, &b_hodQPed);
   fChain->SetBranchAddress("hodTCal", hodTCal, &b_hodTCal);
   fChain->SetBranchAddress("hodQCal", hodQCal, &b_hodQCal);
   fChain->SetBranchAddress("hodBarTCal", hodBarTCal, &b_hodBarTCal);
   fChain->SetBranchAddress("TOF37", &TOF37, &b_TOF37);
   fChain->SetBranchAddress("Beta37", &Beta37, &b_Beta37);
   fChain->SetBranchAddress("Gamma37", &Gamma37, &b_Gamma37);
   fChain->SetBranchAddress("Ek37", &Ek37, &b_Ek37);
   fChain->SetBranchAddress("TOF713", &TOF713, &b_TOF713);
   fChain->SetBranchAddress("Beta713", &Beta713, &b_Beta713);
   fChain->SetBranchAddress("Gamma713", &Gamma713, &b_Gamma713);
   fChain->SetBranchAddress("Ek713", &Ek713, &b_Ek713);
   fChain->SetBranchAddress("beamFL", &beamFL, &b_beamFL);
   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_vBeam_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_vBeam_fBits);
   fChain->SetBranchAddress("fX", &fX, &b_vBeam_fX);
   fChain->SetBranchAddress("fY", &fY, &b_vBeam_fY);
   fChain->SetBranchAddress("fZ", &fZ, &b_vBeam_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_bdc1Position_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_bdc1Position_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_bdc1Position_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_bdc1Position_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_bdc1Position_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_bdc2Position_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_bdc2Position_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_bdc2Position_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_bdc2Position_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_bdc2Position_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_targetPosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_targetPosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_targetPosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_targetPosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_targetPosition_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_sbtPosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_sbtPosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_sbtPosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_sbtPosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_sbtPosition_fZ);
   fChain->SetBranchAddress("fdc0Angle", &fdc0Angle, &b_fdc0Angle);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_fdc0Position_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_fdc0Position_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_fdc0Position_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_fdc0Position_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_fdc0Position_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_vFDC0_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_vFDC0_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_vFDC0_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_vFDC0_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_vFDC0_fZ);
   fChain->SetBranchAddress("hodNHit", &hodNHit, &b_hodNHit);
   fChain->SetBranchAddress("hodID", hodID, &b_hodID);
   fChain->SetBranchAddress("hodTRaw", hodTRaw, &b_hodTRaw);
   fChain->SetBranchAddress("hodBarQCal", hodBarQCal, &b_hodBarQCal);
   fChain->SetBranchAddress("hodQHit", &hodQHit, &b_hodQHit);
   fChain->SetBranchAddress("hodQHitId", hodQHitId, &b_hodQHitId);
   fChain->SetBranchAddress("hodQHitQSyn", hodQHitQSyn, &b_hodQHitQSyn);
   fChain->SetBranchAddress("hodTHit", &hodTHit, &b_hodTHit);
   fChain->SetBranchAddress("hodTHitId", hodTHitId, &b_hodTHitId);
   fChain->SetBranchAddress("hodTHitTSyn", hodTHitTSyn, &b_hodTHitTSyn);
   fChain->SetBranchAddress("hodHit", &hodHit, &b_hodHit);
   fChain->SetBranchAddress("hodHitId", hodHitId, &b_hodHitId);
   fChain->SetBranchAddress("hodHitT", hodHitT, &b_hodHitT);
   fChain->SetBranchAddress("hodHitQ", hodHitQ, &b_hodHitQ);
   fChain->SetBranchAddress("espriPlasE", &espriPlasE, &b_espriPlasE);
   fChain->SetBranchAddress("espriPlasT", &espriPlasT, &b_espriPlasT);
   fChain->SetBranchAddress("espriNaiE", &espriNaiE, &b_espriNaiE);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_vESPRI_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_vESPRI_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_vESPRI_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_vESPRI_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_vESPRI_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_espriRdcPosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_espriRdcPosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_espriRdcPosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_espriRdcPosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_espriRdcPosition_fZ);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_espriPlasPosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_espriPlasPosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_espriPlasPosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_espriPlasPosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_espriPlasPosition_fZ);
   fChain->SetBranchAddress("espriFL", &espriFL, &b_espriFL);
   fChain->SetBranchAddress("espriLocusAngle", &espriLocusAngle, &b_espriLocusAngle);
   fChain->SetBranchAddress("espriAngle", &espriAngle, &b_espriAngle);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_espriPosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_espriPosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_espriPosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_espriPosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_espriPosition_fZ);
//    fChain->SetBranchAddress("dssdQRaw", dssdQRaw, &b_dssdQRaw);
//    fChain->SetBranchAddress("dssdTRaw", dssdTRaw, &b_dssdTRaw);
   fChain->SetBranchAddress("dssdQPed", dssdQPed, &b_dssdQPed);
//    fChain->SetBranchAddress("csiQRaw", csiQRaw, &b_csiQRaw);
//    fChain->SetBranchAddress("csiTRaw", csiTRaw, &b_csiTRaw);
//    fChain->SetBranchAddress("csiQPed", csiQPed, &b_csiQPed);
   fChain->SetBranchAddress("RefTime1", &RefTime1, &b_RefTime1);
   fChain->SetBranchAddress("RefTime2", &RefTime2, &b_RefTime2);
   fChain->SetBranchAddress("dssdTHit", &dssdTHit, &b_dssdTHit);
   fChain->SetBranchAddress("dssdTHitSide", dssdTHitSide, &b_dssdTHitSide);
   fChain->SetBranchAddress("dssdTHitStripId", dssdTHitStripId, &b_dssdTHitStripId);
   fChain->SetBranchAddress("dssdTHitTCal", dssdTHitTCal, &b_dssdTHitTCal);
   fChain->SetBranchAddress("csiTHit", &csiTHit, &b_csiTHit);
   fChain->SetBranchAddress("csiTHitId", csiTHitId, &b_csiTHitId);
   fChain->SetBranchAddress("csiTHitTCal", csiTHitTCal, &b_csiTHitTCal);
   fChain->SetBranchAddress("teleHit", &teleHit, &b_teleHit);
   fChain->SetBranchAddress("teleHitSide", teleHitSide, &b_teleHitSide);
   fChain->SetBranchAddress("teleHitFid", teleHitFid, &b_teleHitFid);
   fChain->SetBranchAddress("teleHitBid", teleHitBid, &b_teleHitBid);
   fChain->SetBranchAddress("teleHitDssdFQPed", teleHitDssdFQPed, &b_teleHitDssdFQPed);
   fChain->SetBranchAddress("teleHitDssdBQPed", teleHitDssdBQPed, &b_teleHitDssdBQPed);
   fChain->SetBranchAddress("teleHitDssdTCal", teleHitDssdTCal, &b_teleHitDssdTCal);
   fChain->SetBranchAddress("teleHitCid", teleHitCid, &b_teleHitCid);
   fChain->SetBranchAddress("teleHitCsiQPed", teleHitCsiQPed, &b_teleHitCsiQPed);
   fChain->SetBranchAddress("teleHitCsiTCal", teleHitCsiTCal, &b_teleHitCsiTCal);
   fChain->SetBranchAddress("teleHitDssdFE", teleHitDssdFE, &b_teleHitDssdFE);
   fChain->SetBranchAddress("teleHitDssdBE", teleHitDssdBE, &b_teleHitDssdBE);
   fChain->SetBranchAddress("teleHitCsiE", teleHitCsiE, &b_teleHitCsiE);
   fChain->SetBranchAddress("bestHit", &bestHit, &b_bestHit);
   fChain->SetBranchAddress("teleSide", &teleSide, &b_teleSide);
   fChain->SetBranchAddress("teleCsiId", &teleCsiId, &b_teleCsiId);
   fChain->SetBranchAddress("teleDssdFid", &teleDssdFid, &b_teleDssdFid);
   fChain->SetBranchAddress("teleDssdBid", &teleDssdBid, &b_teleDssdBid);
   fChain->SetBranchAddress("teleCsiQPed", &teleCsiQPed, &b_teleCsiQPed);
   fChain->SetBranchAddress("teleDssdFQPed", &teleDssdFQPed, &b_teleDssdFQPed);
   fChain->SetBranchAddress("teleDssdBQPed", &teleDssdBQPed, &b_teleDssdBQPed);
   fChain->SetBranchAddress("teleCsiE", &teleCsiE, &b_teleCsiE);
   fChain->SetBranchAddress("teleDssdFE", &teleDssdFE, &b_teleDssdFE);
   fChain->SetBranchAddress("teleDssdBE", &teleDssdBE, &b_teleDssdBE);
   fChain->SetBranchAddress("teleDssdE", &teleDssdE, &b_teleDssdE);
   fChain->SetBranchAddress("teleDssdMaxE", &teleDssdMaxE, &b_teleDssdMaxE);
   fChain->SetBranchAddress("teleCsiT", &teleCsiT, &b_teleCsiT);
   fChain->SetBranchAddress("teleDssdT", &teleDssdT, &b_teleDssdT);
   fChain->SetBranchAddress("teleEnergy", &teleEnergy, &b_teleEnergy);
   fChain->SetBranchAddress("teleAngle", &teleAngle, &b_teleAngle);
   fChain->SetBranchAddress("teleLocusAngle", &teleLocusAngle, &b_teleLocusAngle);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_telePosition_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_telePosition_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_telePosition_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_telePosition_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_telePosition_fZ);
   fChain->SetBranchAddress("telePhi", &telePhi, &b_telePhi);
   fChain->SetBranchAddress("espriPhi", &espriPhi, &b_espriPhi);
   fChain->SetBranchAddress("fdc0Phi", &fdc0Phi, &b_fdc0Phi);
   fChain->SetBranchAddress("beamEnergy", &beamEnergy, &b_beamEnergy);
   fChain->SetBranchAddress("beamBeta", &beamBeta, &b_beamBeta);
   fChain->SetBranchAddress("tofSBTTarget", &tofSBTTarget, &b_tofSBTTarget);
//    fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_vTele_fUniqueID);
//    fChain->SetBranchAddress("fBits", &fBits, &b_vTele_fBits);
//    fChain->SetBranchAddress("fX", &fX, &b_vTele_fX);
//    fChain->SetBranchAddress("fY", &fY, &b_vTele_fY);
//    fChain->SetBranchAddress("fZ", &fZ, &b_vTele_fZ);
//    fChain->SetBranchAddress("beamEnergy", &beamEnergy, &b_beamEnergy);
//    fChain->SetBranchAddress("beamBeta", &beamBeta, &b_beamBeta);
//    fChain->SetBranchAddress("tofSBTTarget", &tofSBTTarget, &b_tofSBTTarget);
   fChain->SetBranchAddress("ppPlasEPlas", &ppPlasEPlas, &b_ppPlasEPlas);
   fChain->SetBranchAddress("ppWinEPlas", &ppWinEPlas, &b_ppWinEPlas);
   fChain->SetBranchAddress("ppShtEPlas", &ppShtEPlas, &b_ppShtEPlas);
   fChain->SetBranchAddress("ppDegEPlas", &ppDegEPlas, &b_ppDegEPlas);
   fChain->SetBranchAddress("ppDegThPlas", &ppDegThPlas, &b_ppDegThPlas);
   fChain->SetBranchAddress("plasProtonEnergy", &plasProtonEnergy, &b_plasProtonEnergy);
   fChain->SetBranchAddress("ppPlasENai", &ppPlasENai, &b_ppPlasENai);
   fChain->SetBranchAddress("ppWinENai", &ppWinENai, &b_ppWinENai);
   fChain->SetBranchAddress("ppShtENai", &ppShtENai, &b_ppShtENai);
   fChain->SetBranchAddress("ppDegENai", &ppDegENai, &b_ppDegENai);
   fChain->SetBranchAddress("ppDegThNai", &ppDegThNai, &b_ppDegThNai);
   fChain->SetBranchAddress("naiEnergy", &naiEnergy, &b_naiEnergy);
   fChain->SetBranchAddress("naiProtonEnergy", &naiProtonEnergy, &b_naiProtonEnergy);
   fChain->SetBranchAddress("protonEnergy", &protonEnergy, &b_protonEnergy);
   fChain->SetBranchAddress("protonTheta", &protonTheta, &b_protonTheta);
   fChain->SetBranchAddress("protonPhi", &protonPhi, &b_protonPhi);
   fChain->SetBranchAddress("Ma", &Ma, &b_Ma);
   fChain->SetBranchAddress("Ea", &Ea, &b_Ea);
   fChain->SetBranchAddress("Pa", &Pa, &b_Pa);
   fChain->SetBranchAddress("Ta", &Ta, &b_Ta);
   fChain->SetBranchAddress("thetaa", &thetaa, &b_thetaa);
   fChain->SetBranchAddress("phia", &phia, &b_phia);
   fChain->SetBranchAddress("Mb", &Mb, &b_Mb);
   fChain->SetBranchAddress("Eb", &Eb, &b_Eb);
   fChain->SetBranchAddress("Pb", &Pb, &b_Pb);
   fChain->SetBranchAddress("Tb", &Tb, &b_Tb);
   fChain->SetBranchAddress("thetab", &thetab, &b_thetab);
   fChain->SetBranchAddress("phib", &phib, &b_phib);
   fChain->SetBranchAddress("MR0", &MR0, &b_MR0);
   fChain->SetBranchAddress("ER0", &ER0, &b_ER0);
   fChain->SetBranchAddress("PR0", &PR0, &b_PR0);
   fChain->SetBranchAddress("TR0", &TR0, &b_TR0);
   fChain->SetBranchAddress("thetaR0", &thetaR0, &b_thetaR0);
   fChain->SetBranchAddress("phiR0", &phiR0, &b_phiR0);
   fChain->SetBranchAddress("M1", &M1, &b_M1);
   fChain->SetBranchAddress("E1", &E1, &b_E1);
   fChain->SetBranchAddress("P1", &P1, &b_P1);
   fChain->SetBranchAddress("T1", &T1, &b_T1);
   fChain->SetBranchAddress("theta1", &theta1, &b_theta1);
   fChain->SetBranchAddress("phi1", &phi1, &b_phi1);
   fChain->SetBranchAddress("M2", &M2, &b_M2);
   fChain->SetBranchAddress("E2", &E2, &b_E2);
   fChain->SetBranchAddress("P2", &P2, &b_P2);
   fChain->SetBranchAddress("T2", &T2, &b_T2);
   fChain->SetBranchAddress("theta2", &theta2, &b_theta2);
   fChain->SetBranchAddress("phi2", &phi2, &b_phi2);
   fChain->SetBranchAddress("exEnergy", &exEnergy, &b_exEnergy);
   fChain->SetBranchAddress("phi12", &phi12, &b_phi12);
   Notify();
}

bool espri::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be for a new TTree in a TChain. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void espri::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t espri::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef espri_cxx
