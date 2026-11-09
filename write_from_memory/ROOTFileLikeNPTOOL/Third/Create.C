
#include "CreateTree.h"
#include "CreatNPToolTree.h"
#include "ReadTree.h"
#include "TFile.h"
#include "TString.h"
#include <iostream>

void Create()
{
    TString OutputRootPath = "Before.root";
    TFile *outputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");

    CreateNPToolTree *detector = new CreateNPToolTree(outputFile);
    CreateTree *reconstruction = new CreateTree(outputFile);

    for (Int_t event = 0; event < 10; event++) {
        detector->Initialize();
        reconstruction->Initialize();

        detector->bdc1->Energy = 10.0 + event;
        detector->bdc1->GetPosition()->SetXYZ(1.0 * event, 2.0 * event, -100.0);
        detector->bdc2->Energy = 20.0 + event;
        detector->bdc2->GetPosition()->SetXYZ(3.0 * event, 4.0 * event, 100.0);

        reconstruction->Energy = 30.0 + event;
        reconstruction->Theta = 0.1 * event;

        detector->Fill();
        reconstruction->Fill();
    }

    outputFile->cd();
    detector->GetTree()->Write();
    reconstruction->GetTree()->Write();

    delete detector;
    delete reconstruction;
    outputFile->Close();
    delete outputFile;
}




void ReadBefore()
{
    TString InputRootPath = "Before.root";
    TFile *inputFile = TFile::Open(InputRootPath.Data(), "READ");

    ReadTree *reader = new ReadTree(inputFile);
    for (Long64_t entry = 0; entry < 2; entry++) {
        reader->GetEntry(entry);

        std::cout << "Entry " << entry
                  << ": BDC1 Energy=" << reader->bdc1->Energy
                  << ", Position=(" << reader->bdc1->GetPosition()->X()
                  << ", " << reader->bdc1->GetPosition()->Y()
                  << ", " << reader->bdc1->GetPosition()->Z() << ")"
                  << "; BDC2 Energy=" << reader->bdc2->Energy
                  << ", Position=(" << reader->bdc2->GetPosition()->X()
                  << ", " << reader->bdc2->GetPosition()->Y()
                  << ", " << reader->bdc2->GetPosition()->Z() << ")"
                  << "; Reconstruction Energy=" << reader->Energy
                  << ", Theta=" << reader->Theta
                  << ", Momentum=" <<reader->GetMomentum()
                  << ", Hit= " << reader->bdc1->GetBDC1Hits() <<std::endl;
    }

    delete reader;
    inputFile->Close();
    delete inputFile;
}
