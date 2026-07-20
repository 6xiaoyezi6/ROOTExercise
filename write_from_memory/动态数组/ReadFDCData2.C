#include <TBranch.h>
#include <TFile.h>
#include <TInterpreter.h>
#include <TTree.h>

#include <iostream>
#include <vector>

#include "ReadFDCData2.h"

//一个 FDC0Hit 结构体对象对应一个 hit；一个事件中的全部 hit由std::vector<FDC0Hit> FDC0_hits;保存
//动态数组嵌套结构体，结构体负责保存一个 hit 的关联信息，而 vector<FDC0Hit> 负责保存一个事件中的所有 hit
//结构是
/*
TTree
└── Event 0
    └── FDC0_hits
        ├── FDC0Hit 0
        ├── FDC0Hit 1
        └── FDC0Hit 2
└── Event 1
    └── FDC0_hits
        ├── FDC0Hit 0
        └── FDC0Hit 1
*/

//对应产生输出root文件，其实相当于把多个动态数组合并成了一个结构体
  // 最重要的层级关系是：

  // 一个 Entry = 一个事件
  // 一个 FDC0_hits[i] = 该事件中的一个 hit
  // FDC0_hits.@size = 该事件的 hit 总数
void ReadFDCData2() {
  // 为自定义结构体和 vector<FDC0Hit> 生成 ROOT 字典。
  gInterpreter->GenerateDictionary("FDC0Hit;std::vector<FDC0Hit>","ReadFDCData2.h;vector");

  TFile *inputFile = TFile::Open("FDCData.root", "READ");
  TTree *inputTree = (TTree *)inputFile->Get("tree");

  Int_t eventID = 0;

  std::vector<Int_t> *FDC0_layerid = nullptr;
  std::vector<Int_t> *FDC0_wireid = nullptr;
  std::vector<Double_t> *FDC0_TDC = nullptr;
  std::vector<Int_t> *FDC0_layer_nhits = nullptr;

  inputTree->SetBranchAddress("eventID", &eventID);
  inputTree->SetBranchAddress("FDC0_layerid", &FDC0_layerid);
  inputTree->SetBranchAddress("FDC0_wireid", &FDC0_wireid);
  inputTree->SetBranchAddress("FDC0_TDC", &FDC0_TDC);
  inputTree->SetBranchAddress("FDC0_layer_nhits", &FDC0_layer_nhits);

  TFile *outputFile = new TFile("ConvertFDCData2.root", "RECREATE");
  TTree *outputTree = new TTree("tree", "FDC0 data stored as vector<FDC0Hit>");

  std::vector<FDC0Hit> FDC0_hits;

  outputTree->Branch("eventID", &eventID, "eventID/I");
  outputTree->Branch("FDC0_hits", &FDC0_hits);

  Long64_t nentries = inputTree->GetEntriesFast();
  //遍历所有事件
  for (Long64_t i = 0; i < nentries; i++) {
    inputTree->GetEntry(i);
    FDC0_hits.clear();

    if (i < 5) {
      std::cout << "Event " << i << std::endl;
      std::cout << "  Number of elements: " << FDC0_TDC->size() << std::endl;
    }

    //遍历当前事件中的所有hit
    for (Int_t hit = 0; hit < (Int_t)FDC0_TDC->size(); hit++) {
      FDC0Hit fdc0Hit;//创建一个临时结构体对象
      fdc0Hit.eventID = eventID;//填入当前 hit 的数据到结构体
      fdc0Hit.layerID = FDC0_layerid->at(hit);
      fdc0Hit.hit = hit;//FDC0_layerid vector 的第 hit 个元素
      fdc0Hit.wireID = FDC0_wireid->at(hit);
      fdc0Hit.TDC = FDC0_TDC->at(hit);

      FDC0_hits.push_back(fdc0Hit);//将结构体加入 vector
    }

    // 只打印第 0 个事件中的全部 hit。
    if (i == 0) {
      for (Int_t index = 0; index < (Int_t)FDC0_hits.size(); index++) //逐个读取动态数组的每一个元素，动态数组的每一个元素都是结构体
      {
        std::cout << "  eventID = " << FDC0_hits[index].eventID
                  << ", layerID = " << FDC0_hits[index].layerID
                  << ", hit = " << FDC0_hits[index].hit
                  << ", wireID = " << FDC0_hits[index].wireID
                  << ", TDC = " << FDC0_hits[index].TDC << std::endl;
      }
    }

    outputTree->Fill();
  }

  outputFile->cd();
  outputTree->Write();
  outputFile->Close();
  inputFile->Close();
}

/*
  应当这样读产生的ROOT文件：

   显示项     含义
  ━━━━━━━━━  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   @size      每个事件中 FDC0_hits 的元素数量
  ─────────  ─────────────────────────────────
   eventID    每个 hit 所属的事件号
  ─────────  ─────────────────────────────────
   layerID    每个 hit 所属层
  ─────────  ─────────────────────────────────
   hit        当前事件内的全局 hit 索引
  ─────────  ─────────────────────────────────
   wireID     发生信号的丝号
  ─────────  ─────────────────────────────────
   TDC        该 hit 对应的 TDC

*/