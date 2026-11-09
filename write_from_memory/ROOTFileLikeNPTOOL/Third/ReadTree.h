/*
 * 功能：读取已有 ROOT 文件的 tree（BDC1、BDC2）和 ReconstructionTree
 *       （Energy、Theta），通过成员变量访问当前事件的数据。
 * 方法：包含 CreateNPToolTreeInitial.h 中的 BDC1、BDC2，以 bdc1、bdc2
 *       指针绑定对象分支，标量直接绑定；GetEntry 读取前重置为 -999，再按
 *       相同条目号读取两棵树。
 * 注意事项：调用者保证文件、树、分支及读取条目有效，分支类型与数据类一致。
 *           输入文件由调用者打开和关闭，必须先删除读取器再关闭文件。
 *           两棵树必须来自同一事件序列且条目数相等；绑定期间不能复制或移动。
 *           需要加载 BDC1、BDC2 的 ROOT 字典；本类不创建或修改输入分支。
 */
#ifndef SECOND_READ_TREE_H
#define SECOND_READ_TREE_H

#include "CreateNPToolTreeInitial.h"
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

    //这个要在ReadTree.h中设置Getter函数，但是NPTOOL那种类型的在Create中创建即可
    Double_t GetMomentum() {return Momentum;}

private:
    TTree *tree;
    TTree *reconstructionTree;

    ReadTree(ReadTree &) = delete;
    ReadTree &operator=(ReadTree &) = delete;
};

#endif
