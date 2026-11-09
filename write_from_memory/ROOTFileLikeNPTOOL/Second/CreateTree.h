/*
 * 功能：创建普通的 ReconstructionTree，管理 Double_t Energy、Theta 分支。
 * 方法：构造时创建树并绑定成员变量；Initialize 重置数据，Fill 保存一个事件。
 * 注意事项：调用者提供有效的可写文件。数据初值为 -999；每个事件赋值前调用
 *           Initialize。树归输出文件管理，
 *           本对象必须在输出文件关闭前删除，且绑定期间不能复制或移动。
 */
#ifndef SECOND_CREATE_TREE_H
#define SECOND_CREATE_TREE_H

#include "TFile.h"
#include "TTree.h"

class CreateTree {
public:
    Double_t Energy;
    Double_t Theta;

    CreateTree(TFile *outputFile)
    {
        Initialize();
        tree = new TTree("ReconstructionTree", "Reconstructed event data");
        tree->SetDirectory(outputFile);
        tree->Branch("Energy", &Energy, "Energy/D");
        tree->Branch("Theta", &Theta, "Theta/D");
    }

    ~CreateTree()
    {
        tree->ResetBranchAddresses();
    }

    void Initialize()
    {
        Double_t kInvalidValue = -999.0;
        Energy = kInvalidValue;
        Theta = kInvalidValue;
    }

    Int_t Fill() {return tree->Fill();}

    TTree *GetTree() {return tree;}

private:
    TTree *tree;

    CreateTree(CreateTree &) = delete;
    CreateTree &operator=(CreateTree &) = delete;
};

#endif
