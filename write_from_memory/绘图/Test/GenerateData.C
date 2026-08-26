/*
 * 功能：生成四组激发能高斯混合样本，并写入 GeneratedEx.root 中名为 tree 的 Ex1、Ex2、Ex3、Ex4 四个 vector<double> 分支。
 * 方法：使用固定种子的 TRandom3，在每个分支给定的峰中心之间等概率抽样，再以 1 MeV 标准差生成高斯随机数；Tree 仅填充一次。
 * 注意事项：四个分支的向量长度依次为 30000、500、30000、500；高斯尾部不截断，绘图宏仅显示指定横轴范围内的数据。
 */

#include "TError.h"
#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

#include <vector>

namespace {

const char *kOutputFile =
    "/Users/yemingxin/ROOT_Exercise/write_from_memory/绘图/Test/GeneratedEx.root";
constexpr UInt_t kRandomSeed = 20260825;
constexpr Double_t kSigmaMeV = 1.0;
constexpr Int_t kEx1Entries = 30000;
constexpr Int_t kEx2Entries = 500;
constexpr Int_t kEx3Entries = 30000;
constexpr Int_t kEx4Entries = 500;
constexpr Double_t kEx12Means[] = {0.0, 3.37, 6.17, 7.54};
constexpr Double_t kEx34Means[] = {10.57, 11.76, 14.0, 16.0, 18.0};

// 按等概率选择峰中心，并向目标向量填入指定数量的高斯随机样本。
void FillGaussianMixture(std::vector<Double_t> &values, Int_t entryCount,
                         const Double_t *means, Int_t meanCount,
                         TRandom3 &random)
{
    values.clear();
    values.reserve(entryCount);
    for (Int_t entry = 0; entry < entryCount; ++entry) {
        const Int_t peakIndex = random.Integer(meanCount);
        values.push_back(random.Gaus(means[peakIndex], kSigmaMeV));
    }
}

} // namespace

// 生成四个不同长度的激发能样本向量，并写入单条 Tree 记录。
void GenerateData()
{
    TRandom3 random(kRandomSeed);
    std::vector<Double_t> ex1;
    std::vector<Double_t> ex2;
    std::vector<Double_t> ex3;
    std::vector<Double_t> ex4;

    FillGaussianMixture(ex1, kEx1Entries, kEx12Means, 4, random);
    FillGaussianMixture(ex2, kEx2Entries, kEx12Means, 4, random);
    FillGaussianMixture(ex3, kEx3Entries, kEx34Means, 5, random);
    FillGaussianMixture(ex4, kEx4Entries, kEx34Means, 5, random);

    TFile outputFile(kOutputFile, "RECREATE");
    TTree tree("tree", "Generated excitation-energy samples");
    tree.Branch("Ex1", &ex1);
    tree.Branch("Ex2", &ex2);
    tree.Branch("Ex3", &ex3);
    tree.Branch("Ex4", &ex4);
    tree.Fill();
    tree.Write();
    outputFile.Close();

    Info("GenerateData",
         "已写入 %s：Ex1=%zu, Ex2=%zu, Ex3=%zu, Ex4=%zu",
         kOutputFile, ex1.size(), ex2.size(), ex3.size(), ex4.size());
}
