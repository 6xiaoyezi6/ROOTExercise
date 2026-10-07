#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"

void GenerateVectorData() {

    TFile *file = new TFile("VectorData.root", "RECREATE");
    TTree *tree = new TTree("tree", "simple vector tree");
    // 定义 branch 对应的变量
    Int_t EventID;
    TVector3 Position;

    // 创建 branch
    tree->Branch("EventID", &EventID);
    tree->Branch("Position", &Position);

    // 填充 10 个事件
    for (Int_t i = 0; i < 10; i++) 
    {
        EventID = i;
        Position.SetXYZ(1.0 * i,2.0 * i,3.0 * i);
        tree->Fill();
    }
    tree->Write();
    file->Close();
}