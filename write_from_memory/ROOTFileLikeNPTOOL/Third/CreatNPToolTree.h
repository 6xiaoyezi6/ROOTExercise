#ifndef SECOND_CREAT_NPTOOL_TREE_H
#define SECOND_CREAT_NPTOOL_TREE_H

#include "CreateNPToolTreeInitial.h"
#include "TFile.h"
#include "TTree.h"

class CreateNPToolTree {
public:
    BDC1 *bdc1;
    BDC2 *bdc2;

    CreateNPToolTree(TFile *outputFile)
    {
        bdc1 = new BDC1();
        bdc2 = new BDC2();
        tree = new TTree("tree", "NPTool like data");
        tree->SetDirectory(outputFile);
        tree->Branch("BDC1", "BDC1", &bdc1, 32000, 99);
        tree->Branch("BDC2", "BDC2", &bdc2, 32000, 99);
    }

    ~CreateNPToolTree()
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

    CreateNPToolTree(CreateNPToolTree &) = delete;
    CreateNPToolTree &operator=(CreateNPToolTree &) = delete;
};

#endif
