#include <TBranch.h>
#include <TFile.h>
#include <TInterpreter.h>
#include <TTree.h>

#include <iostream>
#include <tuple>
#include <vector>
//使用关联容器保存击中信息
// (事件号, 层号, 当前事件内的全局 hit 索引,丝号, TDC)
//std::vector<std::tuple<int, int, int, int, double>> FDC0_hits;
/*
ConvertFDCData1.root 使用 tuple：

  std::vector<std::tuple<int, int, int, int, double>>

  tuple 成员没有名字，只能按照位置访问：

  std::get<0>(hit)  // eventID
  std::get<1>(hit)  // layerID
  std::get<2>(hit)  // globalHitIndex
  std::get<3>(hit)  // wireID
  std::get<4>(hit)  // TDC

  所以 ROOT 中显示为：

  FDC0_hits._0
  FDC0_hits._1
  FDC0_hits._2
  FDC0_hits._3
  FDC0_hits._4

  ConvertFDCData2.root 使用结构体：

  struct FDC0Hit {
    Int_t eventID;
    Int_t layerID;
    Int_t hit;
    Int_t wireID;
    Double_t TDC;
  };

  std::vector<FDC0Hit>

  成员有明确名字：

  hit.eventID
  hit.layerID
  hit.hit
  hit.wireID
  hit.TDC

  ROOT 中也显示为：

  FDC0_hits.eventID
  FDC0_hits.layerID
  FDC0_hits.hit
  FDC0_hits.wireID
  FDC0_hits.TDC
*/

void ReadFDCData1() {
  gInterpreter->GenerateDictionary(
      "std::vector<std::tuple<int,int,int,int,double>>",
      "vector;tuple");

  TFile *f = TFile::Open("FDCData.root", "READ");
  TTree *tree = (TTree *)f->Get("tree");

  Int_t eventID = 0;

  //指针
  std::vector<Int_t> *FDC0_layerid = nullptr;
  std::vector<Int_t> *FDC0_wireid = nullptr;
  std::vector<Double_t> *FDC0_TDC = nullptr;
  std::vector<Int_t> *FDC0_layer_nhits = nullptr;

  tree->SetBranchAddress("eventID", &eventID);
  tree->SetBranchAddress("FDC0_layerid", &FDC0_layerid);
  tree->SetBranchAddress("FDC0_wireid", &FDC0_wireid);
  tree->SetBranchAddress("FDC0_TDC", &FDC0_TDC);
  tree->SetBranchAddress("FDC0_layer_nhits", &FDC0_layer_nhits);

  // 每一个 tuple 保存五个相互对应的数据：
  // (事件号, 层号, 当前事件内的全局 hit 索引,丝号, TDC)
  std::vector<std::tuple<int, int, int, int, double>> FDC0_hits;

  TFile *outputFile = new TFile("ConvertFDCData1.root", "RECREATE");
  TTree *outputTree = new TTree(
      "tree", "FDC0 data stored as vector<tuple<int,int,int,int,double>>");

  outputTree->Branch("eventID", &eventID, "eventID/I");
  outputTree->Branch("FDC0_hits", &FDC0_hits);

  //遍历所有事件
  Long64_t nentries = tree->GetEntriesFast();
  for (Long64_t i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    FDC0_hits.clear();

    //遍历当前事件中的所有hit
    for (Int_t hit = 0; hit < (Int_t)FDC0_TDC->size(); hit++) {
      FDC0_hits.emplace_back(eventID,
                             FDC0_layerid->at(hit),
                             hit,
                             FDC0_wireid->at(hit),
                             FDC0_TDC->at(hit));
    }

    if (i < 5) 
    {
      std::cout << "Event " << i << std::endl;
      std::cout << "  Number of elements: " << FDC0_TDC->size() << std::endl;
    }

    // 只把第 0 个事件的数据逐个输出。
    if (i == 0) {
      for (Int_t index = 0; index < (Int_t)FDC0_hits.size(); index++) {
        Int_t tupleEventID = std::get<0>(FDC0_hits[index]);
        Int_t tupleLayerID = std::get<1>(FDC0_hits[index]);
        Int_t tupleHit = std::get<2>(FDC0_hits[index]);
        Int_t tupleWireID = std::get<3>(FDC0_hits[index]);
        Double_t tupleTDC = std::get<4>(FDC0_hits[index]);

        std::cout << "  eventID = " << tupleEventID
                  << ", layerID = " << tupleLayerID
                  << ", hit = " << tupleHit
                  << ", wireID = " << tupleWireID
                  << ", TDC = " << tupleTDC << std::endl;
      }
    }

    outputTree->Fill();
  }

  outputFile->cd();
  outputTree->Write();
  outputFile->Close();
  f->Close();
}
