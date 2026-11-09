/*
 * 功能：创建类似 NPTool 的 tree，管理独立的 BDC1、BDC2 对象分支及事件填充。
 * 方法：包含 CreateNPToolTreeInitial.h 中的 BDC1、BDC2 类，以 bdc1、bdc2
 *       指针持有对象并以拆分层级 99 绑定分支；Initialize 初始化，Fill 保存事件。
 * 注意事项：调用者提供有效的可写文件；树归文件管理，本对象须在文件关闭前
 *           删除，绑定期间不能复制或移动。新增分支时补充对象分配、Branch、
 *           Initialize 和释放代码；使用 ACLiC 加载以生成数据类字典。
 */
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
