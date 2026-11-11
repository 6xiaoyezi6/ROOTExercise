#ifndef READ_TREE_H
#define READ_TREE_H

#include "CreateObjectTreeInitial.h"
#include "TFile.h"
#include "TTree.h"

class ReadTree {
public:
    BDC1 *bdc1;
    BDC2 *bdc2;
    Double_t Energy;
    Double_t Theta;
    Double_t Momentum;

    ReadTree(TFile *inputFile)
    {
        tree = (TTree*)inputFile->Get("tree");
        reconstructionTree = (TTree*)inputFile->Get("ReconstructionTree");

        bdc1 = new BDC1();
        bdc2 = new BDC2();
        Initialize();

        tree->SetBranchAddress("BDC1", &bdc1);
        tree->SetBranchAddress("BDC2", &bdc2);
        reconstructionTree->SetBranchAddress("Energy", &Energy);
        reconstructionTree->SetBranchAddress("Theta", &Theta);
        reconstructionTree->SetBranchAddress("Momentum",&Momentum);
    }

    ~ReadTree()
    {
        tree->ResetBranchAddresses();
        reconstructionTree->ResetBranchAddresses();
        delete bdc1;
        delete bdc2;
    }

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        bdc1->Initialize();
        bdc2->Initialize();
        Energy = kInvalidValue;
        Theta = kInvalidValue;
        Momentum = kInvalidValue;
    }

    Long64_t GetEntries() {return tree->GetEntries();}

    // 按相同条目号读取两棵树，返回读取的总字节数。
    Long64_t GetEntry(Long64_t entry)
    {
        Initialize();
        Long64_t detectorBytes = tree->GetEntry(entry);
        Long64_t reconstructionBytes = reconstructionTree->GetEntry(entry);
        return detectorBytes + reconstructionBytes;
    }

    TTree *GetTree() {return tree;}
    TTree *GetReconstructionTree() {return reconstructionTree;}

    //这个要在ReadTree.h中设置Getter函数，但是对象分支的Getter函数在数据类中定义即可
    Double_t GetMomentum() {return Momentum;}

private:
    TTree *tree;
    TTree *reconstructionTree;

    ReadTree(ReadTree &) = delete;
    ReadTree &operator=(ReadTree &) = delete;
};

#endif
