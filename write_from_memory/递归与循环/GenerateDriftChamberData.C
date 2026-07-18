#include <tuple>
#include <vector>

#include "TFile.h"
#include "TInterpreter.h"
#include "TRandom3.h"
#include "TTree.h"
//第一个void函数用于生成坐标对（x,y）
// 根据直线参数生成一个事件的漂移室击中坐标。
void GenerateOneEvent(std::vector<std::tuple<double, double>>& hits,
                      int nHits, double slope, double intercept)
{
    TRandom3 random(0);
    hits.clear();

    for (int iHit = 0; iHit < nHits; ++iHit) {
        double x = random.Uniform(-50.0, 50.0);
        double y = slope * x + intercept + random.Gaus(1, 2)+random.Poisson(3.0);
        hits.emplace_back(x, y);
    }
}

//对每一个事件按照有0对，1对，2对，3对，4对(x,y)坐标对进行填充
// 生成多个漂移室事件并保存到 ROOT 文件。
void GenerateDriftChamberData(int nEvents = 10000)
{
    // 为击中坐标容器生成 ROOT 字典，使其能够写入 TTree
    //让复杂的数据类型vector<std::tuple<double, double>>& hits直接填入树中
    //GenerateDictionary("需要生成字典的类型","定义这些类型所需的头文件");让 ROOT 认识这种复杂类型
    gInterpreter->GenerateDictionary("vector<tuple<double,double>>", "vector;tuple");

    TRandom3 random(0);
    TFile*f = new TFile("DriftChamberData.root", "RECREATE");
    TTree* tree = new TTree("tree", "Simulated drift chamber hits");

    int eventID = 0;
    int nHits = 0;
    double trueSlope = 0.0;
    double trueIntercept = 0.0;
    std::vector<std::tuple<double, double>> hits;

    tree->Branch("eventID", &eventID);
    tree->Branch("nHits", &nHits);
    tree->Branch("trueSlope", &trueSlope);
    tree->Branch("trueIntercept", &trueIntercept);
    tree->Branch("x1", &hits);

    for (eventID = 0; eventID < nEvents; ++eventID) {
        nHits = random.Integer(5);
        trueSlope = random.Uniform(-1.0, 1.0);
        trueIntercept = random.Uniform(-10.0, 10.0);

        GenerateOneEvent(hits, nHits, trueSlope, trueIntercept);
        tree->Fill();
    }

    tree->Write();
    f->Close();
}

/*
ROOT 将 std::vector<std::tuple<double, double>> 拆分成了三个部分：

  - @size：当前事件中 vector 的元素数量，也就是击中点数量。
  - x1._0：每个 tuple 的第 0 个元素，即 x 坐标。
  - x1._1：每个 tuple 的第 1 个元素，即 y 坐标。

  例如某个事件中：

  hits = {
      std::make_tuple(10.0, 3.2),
      std::make_tuple(20.0, 5.1),
      std::make_tuple(30.0, 7.4)
  };

  ROOT 中对应为：

  @size = 3
  x1._0 = [10.0, 20.0, 30.0]
  x1._1 = [ 3.2,  5.1,  7.4]

  因此，同一位置的两个元素组成一个坐标对：

  (x1._0[0], x1._1[0]) → 第一个 (x, y)
  (x1._0[1], x1._1[1]) → 第二个 (x, y)


*/