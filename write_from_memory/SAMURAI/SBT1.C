#include <iostream>
#include <unistd.h>

#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TDatime.h"
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"

#include "TArtBigRIPSParameters.hh"
#include "TArtCalibPlastic.hh"
#include "TArtEventStore.hh"
#include "TArtPlastic.hh"

void SBT1(int runnumber = 0 , int MaxEv = 20000)
{
    // =========================================================
    // Create timestamp output directory
    // =========================================================
    TDatime now;

    const TString outdirBase = "output/checkplastic";
    const TString timestamp = Form("%04d%02d%02d_%02d%02d",
                                   now.GetYear(), now.GetMonth(), now.GetDay(),
                                   now.GetHour(), now.GetMinute());

    const TString outdir = Form("%s/%s", outdirBase.Data(), timestamp.Data());

    gSystem->mkdir(outdir.Data());


    // =========================================================
    // Load plastic calibration parameter
    // =========================================================
    TArtBigRIPSParameters *fbrprm = TArtBigRIPSParameters::Instance();
    fbrprm->LoadParameter("./db/SAMURAIPlastic.20260519.xml");

    TArtCalibPlastic *fCalibPlastic = new TArtCalibPlastic();

    // =========================================================
    // Open RIDF file
    // =========================================================
    TArtEventStore *festore = new TArtEventStore();
    TString filename = Form("/Users/yemingxin/anaroot/ridf/run%04d.ridf", runnumber);
    festore->Open(filename.Data());

    // =========================================================
    // Output ROOT file
    // =========================================================
    TFile *outputFile = new TFile(Form("%s/f13_1_qraw_run%04d.root", outdir.Data(), runnumber),"RECREATE");


    TTree *tree = new TTree("tree", "tree");
    TClonesArray *pla = new TClonesArray("TArtPlastic", 1000);

    Double_t f13_1ql, f13_1qr, f13_1qave;
    Double_t f13_1tl, f13_1tr, f13_1t;

    tree->Branch("pla", &pla);
    tree->Branch("f13_1ql", &f13_1ql, "f13_1ql/D");
    tree->Branch("f13_1qr", &f13_1qr, "f13_1qr/D");
    tree->Branch("f13_1qave", &f13_1qave, "f13_1qave/D");
    tree->Branch("f13_1tl", &f13_1tl, "f13_1tl/D");
    tree->Branch("f13_1tr", &f13_1tr, "f13_1tr/D");
    tree->Branch("f13_1t", &f13_1t, "f13_1t/D");

    // =========================================================
    // Histograms: only F13-1 QLRaw, QRRaw, QAveRaw
    // =========================================================
    const Int_t qraw_bin = 500;
    const Double_t qraw_min = 0.;
    const Double_t qraw_max = 2500.;

    TH1 *h_sbt1_qlraw = new TH1I("sbt1_QLRaw","F13-1 / SBT1 QLRaw; QLRaw [ch]; Counts",qraw_bin, qraw_min, qraw_max);

    TH1 *h_sbt1_qrraw = new TH1I("sbt1_QRRaw","F13-1 / SBT1 QRRaw; QRRaw [ch]; Counts",qraw_bin, qraw_min, qraw_max);

    TH1 *h_sbt1_qaveraw = new TH1I("sbt1_QAveRaw","F13-1 / SBT1 QAveRaw; QAveRaw [ch]; Counts",qraw_bin, qraw_min, qraw_max);

    // =========================================================
    // Event loop
    // =========================================================

    Int_t neve = 0;

    while (festore->GetNextEvent() && neve < MaxEv)
    {

        std::cout << "\r event = " << neve << std::flush;

        f13_1ql = f13_1qr = f13_1qave = -9999.;
        f13_1tl = f13_1tr = f13_1t = -9999.;

        fCalibPlastic->ClearData();
        pla->Clear("C");

        fCalibPlastic->ReconstructData();

        TClonesArray *plahits =(TClonesArray *)fCalibPlastic->GetPlasticArray();

        const Int_t nhits_pla = plahits->GetEntries();

        for (Int_t i = 0; i < nhits_pla; ++i)
        {
            TArtPlastic *hit = (TArtPlastic *)plahits->At(i);
            TArtPlastic *out = (TArtPlastic *)pla->ConstructedAt(pla->GetEntriesFast());
            hit->Copy(*out);

            if (hit->GetID() != 4)
            {
                continue;
            }

            h_sbt1_qlraw->Fill(hit->GetQLRaw());
            h_sbt1_qrraw->Fill(hit->GetQRRaw());
            h_sbt1_qaveraw->Fill(hit->GetQAveRaw());

            f13_1ql = hit->GetQLRaw();
            f13_1qr = hit->GetQRRaw();
            f13_1qave = hit->GetQAveRaw();
            f13_1tl = hit->GetTLRaw();
            f13_1tr = hit->GetTRRaw();
            f13_1t = hit->GetTime();
        }

        tree->Fill();

        neve++;
        festore->ClearData();
    }



    // =========================================================
    // Draw and save PDF
    // =========================================================
    TCanvas *c_qraw = new TCanvas("c_qraw", "F13-1 QRaw", 1200, 400);
    c_qraw->Divide(3, 1);

    c_qraw->cd(1);
    h_sbt1_qlraw->Draw();

    c_qraw->cd(2);
    h_sbt1_qrraw->Draw();

    c_qraw->cd(3);
    h_sbt1_qaveraw->Draw();

    c_qraw->Modified();
    c_qraw->Update();

    const TString pdfname = Form("%s/f13_1_qraw_run%04d.pdf",outdir.Data(),runnumber);

    c_qraw->SaveAs(pdfname.Data());

    // =========================================================
    // Write ROOT file
    // =========================================================
    outputFile->cd();

    tree->Write();
    h_sbt1_qlraw->Write();
    h_sbt1_qrraw->Write();
    h_sbt1_qaveraw->Write();

    outputFile->Write();
    outputFile->Close();


}
