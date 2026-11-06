#include "TObject.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TVector3.h"
#include <iostream>
#include "TreeDemo.h"

void TreeDemo()
{
    // 第二步：创建 ROOT 文件和唯一的 TTree。
    TString OutputRootPath = "demo.root";
    TFile *outputFile = TFile::Open(OutputRootPath.Data(), "RECREATE");
    TTree *tree = new TTree("tree", "Object branch demonstration");

    BDC *bdc = new BDC();
    RecoilGAGG *rg = new RecoilGAGG();
    RecoilSSD *rs = new RecoilSSD();

    // 第三步：绑定对象分支。参数依次为分支名、类名、对象指针的地址、缓冲大小、拆分层级。
    // &bdc 是指针变量的地址；指针和对象在整个填树过程中都必须保持有效。
    // split level=99 使成员成为可展开的子分支；0 则不拆分对象成员。
    tree->Branch("BDC", "BDC", &bdc, 32000, 99);
    tree->Branch("RecoilGAGG", "RecoilGAGG", &rg, 32000, 99);
    tree->Branch("RecoilSSD", "RecoilSSD", &rs, 32000, 99);

    // 第四步：逐事件更新三个对象，然后对整棵树 Fill 一次。
    for (Int_t event = 0; event < 10; event++) {
        // 每个事件先初始化；最后一个事件不再赋值，直接保存 -999。
        bdc->Initialize();
        rg->Initialize();
        rs->Initialize();

        if (event < 9) {
            bdc->X = 0.1 * event;
            bdc->Y = -0.2 * event;
            bdc->Z = -1000.0;

            rg->Energy = 10.0 + event;
            rs->Position.SetXYZ(1.0 * event, 2.0 * event, 3.0 * event);
        }

        // Fill 保存三个对象当前的数据；Branch 只需在事件循环前创建一次。
        tree->Fill();
    }

    // 第五步：写入磁盘。文件关闭时会释放它管理的 tree。
    outputFile->cd();
    tree->Write();
    tree->Print();
    outputFile->Close();
    delete outputFile;
    delete bdc;
    delete rg;
    delete rs;

}

void ReadTreeDemo()
{
    TString InputRootPath = "demo.root";
    TFile *inputFile = TFile::Open(InputRootPath.Data(), "READ");
    TTree *tree = (TTree*)inputFile->Get("tree");

    BDC *bdc = new BDC();
    RecoilGAGG *rg = new RecoilGAGG();
    RecoilSSD *rs = new RecoilSSD();

    tree->SetBranchAddress("BDC", &bdc);
    tree->SetBranchAddress("RecoilGAGG", &rg);
    tree->SetBranchAddress("RecoilSSD", &rs);

    // GetEntry 会更新三个对象中的成员，随后就可以通过 -> 访问当前事件。
    for (Long64_t entry = 0; entry < tree->GetEntries(); entry++) {
        tree->GetEntry(entry);
        std::cout << "Entry " << entry << ": BDC X=" << bdc->X
                  << ", Y=" << bdc->Y << ", Z=" << bdc->Z
                  << ", GAGG Energy=" << rg->Energy
                  << ", SSD Position=(" << rs->Position.X()
                  << ", " << rs->Position.Y()
                  << ", " << rs->Position.Z() << ")" << std::endl;
    }

    tree->ResetBranchAddresses();
    inputFile->Close();
    delete inputFile;
    delete bdc;
    delete rg;
    delete rs;
}
