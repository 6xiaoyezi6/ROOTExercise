#include <TFile.h>
#include <TRandom3.h>
#include <TTree.h>

//生成DSSDRawQ[4][32]数据类型
//[4]依次是左前，左后，右前，右后。
//每一个事件只能有一侧响应，该侧的前、后面分别独立产生1-5个命中。
/*
root [10] tree->Scan("Iteration$:DSSDRawQ","DSSDRawQ!=-999","",1,5); //只查看Entry 5

Instance是展开的数组元素编号，范围是0-127。
每个事件会显示2-10行非-999的命中数据。


类似的 ROOT Scan固定特殊语法还有：

Entry$       // 当前事件编号
LocalEntry$  // 当前文件内的事件编号
Iteration$   // 当前数组元素编号
Length$      // 数组长度
Sum$         // 对数组表达式求和
Max$         // 数组最大值
Min$         // 数组最小值



*/
void GenerateDSSDData()
{
  const Int_t nEvents = 10000;
  const Int_t nFaces = 4;
  const Int_t nChannels = 32;
  const Int_t maxMultiplicity = 5;

  // DSSDRawQ[0]: LF, DSSDRawQ[1]: LB,
  // DSSDRawQ[2]: RF, DSSDRawQ[3]: RB.
  Double_t DSSDRawQ[nFaces][nChannels];

  TFile *outputFile = new TFile("DSSDData.root", "RECREATE");
  TTree *tree = new TTree("tree", "DSSD raw charge");
  tree->Branch("DSSDRawQ", DSSDRawQ, "DSSDRawQ[4][32]/D");

  TRandom3 random(0);

  for (Int_t event = 0; event < nEvents; event++) {
    for (Int_t face = 0; face < nFaces; face++) {
      for (Int_t channel = 0; channel < nChannels; channel++) {
        DSSDRawQ[face][channel] = -999;
      }
    }

    // selectedSide = 0: left, selectedSide = 1: right.
    Int_t selectedSide = random.Integer(2);
    Int_t frontFace = 2 * selectedSide;
    Int_t backFace = frontFace + 1;

    Int_t frontMultiplicity = random.Integer(maxMultiplicity) + 1;
    Int_t backMultiplicity = random.Integer(maxMultiplicity) + 1;

    Bool_t usedFrontChannels[nChannels] = {kFALSE};
    Bool_t usedBackChannels[nChannels] = {kFALSE};

    for (Int_t hit = 0; hit < frontMultiplicity; hit++) {
      Int_t frontChannel = random.Integer(nChannels);
      while (usedFrontChannels[frontChannel]) {
        frontChannel = random.Integer(nChannels);
      }
      usedFrontChannels[frontChannel] = kTRUE;
      DSSDRawQ[frontFace][frontChannel] = random.Gaus(1000.0, 10.0);
    }

    for (Int_t hit = 0; hit < backMultiplicity; hit++) {
      Int_t backChannel = random.Integer(nChannels);
      while (usedBackChannels[backChannel]) {
        backChannel = random.Integer(nChannels);
      }
      usedBackChannels[backChannel] = kTRUE;
      DSSDRawQ[backFace][backChannel] = random.Gaus(1000.0, 10.0);
    }

    tree->Fill();
  }

  tree->Write();
  outputFile->Close();
}
