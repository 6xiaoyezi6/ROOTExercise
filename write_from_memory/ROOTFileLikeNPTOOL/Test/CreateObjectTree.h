#ifndef CREATE_OBJECT_TREE_H
#define CREATE_OBJECT_TREE_H

#include "CreateObjectTreeInitial.h"
#include "TFile.h"
#include "TTree.h"

class CreateObjectTree {
public:
    BDC1 *bdc1;
    BDC2 *bdc2;

    CreateObjectTree(TFile *outputFile)
    {
        bdc1 = new BDC1();
        bdc2 = new BDC2();
        tree = new TTree("tree", "Object branch data");
        tree->SetDirectory(outputFile);
        tree->Branch("BDC1", "BDC1", &bdc1, 32000, 99);
        tree->Branch("BDC2", "BDC2", &bdc2, 32000, 99);
    }

    ~CreateObjectTree()
    {
        tree->ResetBranchAddresses();
        delete bdc1;
        delete bdc2;
    }

    void Initialize()
    {
        bdc1->Initialize();
        bdc2->Initialize();
    }

    Int_t Fill() {return tree->Fill();}
    TTree *GetTree() {return tree;}

private:
    TTree *tree;

    CreateObjectTree(CreateObjectTree &) = delete;
    CreateObjectTree &operator=(CreateObjectTree &) = delete;
};

#endif
