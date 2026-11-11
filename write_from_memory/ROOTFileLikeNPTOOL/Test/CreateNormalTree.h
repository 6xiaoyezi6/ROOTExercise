#ifndef CREATE_NORMAL_TREE_H
#define CREATE_NORMAL_TREE_H

#include "TFile.h"
#include "TTree.h"

class CreateNormalTree {
public:
    Double_t Energy;
    Double_t Theta;
    Double_t Momentum;

    CreateNormalTree(TFile *outputFile)
    {
        Initialize();
        tree = new TTree("ReconstructionTree", "Reconstructed event data");
        tree->SetDirectory(outputFile);
        tree->Branch("Energy", &Energy, "Energy/D");
        tree->Branch("Theta", &Theta, "Theta/D");
        tree->Branch("Momentum",&Momentum,"Momentum/D");
    }

    ~CreateNormalTree()
    {
        tree->ResetBranchAddresses();
    }

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        Energy = kInvalidValue;
        Theta = kInvalidValue;
        Momentum = kInvalidValue;
    }

    Int_t Fill() {return tree->Fill();}
    TTree *GetTree() {return tree;}
    

private:
    TTree *tree;

    CreateNormalTree(CreateNormalTree &) = delete;
    CreateNormalTree &operator=(CreateNormalTree &) = delete;
};

#endif
