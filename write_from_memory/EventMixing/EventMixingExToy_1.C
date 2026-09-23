// EventMixingExToy.C
// 模拟静止的 10Be*(10.15 MeV) -> 6He + alpha 两体衰变
// 真实信号事件采用 Breit-Wigner 分布，Gamma = 0.296 MeV
// 真实“本底”事件中 6He 和 alpha 的四动量彼此独立连续采样
// 利用 6He 和 alpha 的四动量通过 invariant mass 重建 10Be 激发能
// 再通过 Event Mixing 方法构造本底，并与真实“本底”比较
//三类事件：真实的物理事件，人为构造的"真实本底"事件，Event Mixing构造的本底事件

//这里使用所有事件（真实事件+真实本底）作为Event Mixing的"事件池"
//就会出现Event Mixing产生的事件与"真实本底"不一样的情况

#include <TCanvas.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TMath.h>
#include <vector>
#include <iostream>
#include <cmath>

const double U = 931.49410242;
const double M10 = 10.013534695 * U;
const double M6 = 6.018885891 * U;
const double MA = 4.00260325413 * U;
const double E0 = 10.15;
const double Gamma = 0.296;
const double Threshold = M6 + MA - M10;

double SampleResonanceEx(TRandom3 &rng)
{
    double Ex;
    do Ex = rng.BreitWigner(E0, Gamma);
    while (Ex <= Threshold || Ex >= 15.0);
    return Ex;
}

double TwoBodyMomentum(double Ex)
{
    double M = M10 + Ex;
    double x = M * M - std::pow(M6 + MA, 2);
    double y = M * M - std::pow(M6 - MA, 2);
    return std::sqrt(x * y) / (2.0 * M);
}

double SampleBackgroundMomentum(TRandom3 &rng, double p0 = 80.0)
{
    double p;
    do p = rng.Exp(p0);
    while (p < 20.0 || p > 220.0);
    return p;
}

TVector3 RandomMomentum(TRandom3 &rng, double p)
{
    double cosTheta = rng.Uniform(-1.0, 1.0);
    double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
    double phi = rng.Uniform(0.0, 2.0 * TMath::Pi());
    return TVector3(p * sinTheta * std::cos(phi), p * sinTheta * std::sin(phi), p * cosTheta);
}

TLorentzVector MakeParticle(const TVector3 &p, double mass)
{
    double E = std::sqrt(p.Mag2() + mass * mass);
    return TLorentzVector(p.X(), p.Y(), p.Z(), E);
}

double ReconstructEx(const TLorentzVector &he6, const TLorentzVector &alpha)
{
    return (he6 + alpha).M() - M10;
}

void EventMixingExToy_1()
{
    gStyle->SetOptStat(0);

    const int N = 200000;
    const double signalFraction = 0.30;
    TRandom3 rng(42);

    std::vector<TLorentzVector> he6List, alphaList;
    he6List.reserve(N); alphaList.reserve(N);

    TH1D *hReal = new TH1D("hReal", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hTrueBG = new TH1D("hTrueBG", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hMixed = new TH1D("hMixed", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);

    hReal->Sumw2(); hTrueBG->Sumw2(); hMixed->Sumw2();

    // ============================================================
    // 1. 构造真实数据
    //    真实数据 = 真实信号事件 + 真实“本底”事件
    // ============================================================
    for (int i = 0; i < N; ++i) {
        bool isSignal = rng.Uniform() < signalFraction;
        TLorentzVector he6, alpha;

        // --------------------------------------------------------
        // 1.1 构造真实信号事件
        //     10Be*(10.15 MeV) -> 6He + alpha
        //     两个子核来自同一次两体衰变，因此严格满足动量守恒
        //     p_6He = -p_alpha
        // --------------------------------------------------------
        if (isSignal) {
            double Ex = SampleResonanceEx(rng);
            double p = TwoBodyMomentum(Ex);
            TVector3 p6 = RandomMomentum(rng, p);

            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(-p6, MA);
        }

        // --------------------------------------------------------
        // 1.2 构造真实“本底”事件
        //     6He 和 alpha 不来自同一个两体衰变
        //     两者的动量大小分别独立从连续分布中采样
        //     两者的方向也分别独立随机产生
        //     因此不存在 p_6He = -p_alpha 的物理关联
        // --------------------------------------------------------
        else {
            double p6mag = SampleBackgroundMomentum(rng);
            double pAmag = SampleBackgroundMomentum(rng);

            TVector3 p6 = RandomMomentum(rng, p6mag);
            TVector3 pA = RandomMomentum(rng, pAmag);

            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(pA, MA);
        }

        // 保存每个事件中的 6He 和 alpha 四动量
        // 后面 Event Mixing 要重新使用这些单粒子信息
        he6List.push_back(he6);
        alphaList.push_back(alpha);

        // 使用同一个事件中的 6He_i 和 alpha_i 重建激发能
        // 这就是 Same Event
        double ExReco = ReconstructEx(he6, alpha);

        // 蓝色谱：真实数据 = Signal + Background
        hReal->Fill(ExReco);

        // 红色谱：只有 toy MC 中我们才知道哪些事件是真实“本底”
        if (!isSignal) hTrueBG->Fill(ExReco);
    }

    // ============================================================
    // 2. 使用 Event Mixing 方法构造本底
    //    Same Event：(6He_i, alpha_i)
    //    Event Mixing:(6He_i, alpha_(i+1))
    //    两个粒子来自不同事件，因此原来的两体物理关联被破坏
    // ============================================================
    for (int i = 0; i < N; ++i) {
        int j = (i + 1) % N;
        double ExMixed = ReconstructEx(he6List[i], alphaList[j]);

        // 绿色谱：Event Mixing 方法得到的本底
        hMixed->Fill(ExMixed);
    }

    // ============================================================
    // 3. 对 Event Mixing 本底进行归一化
    //    Event Mixing 主要给出本底形状，不自动给出真实本底数目
    //    这里使用低激发能侧带 7.42 - 8.50 MeV 做归一化
    // ============================================================
    int b1 = hReal->FindBin(7.42);
    int b2 = hReal->FindBin(8.50 - 1e-6);

    double realSideband = hReal->Integral(b1, b2);
    double mixedSideband = hMixed->Integral(b1, b2);
    double alphaNorm = realSideband / mixedSideband;

    hMixed->Scale(alphaNorm);

    std::cout << "6He + alpha threshold = " << Threshold << " MeV" << std::endl;
    std::cout << "Normalization alpha = " << alphaNorm << std::endl;
    std::cout << "Expected background fraction = " << 1.0 - signalFraction << std::endl;

    // ============================================================
    // 4. 用 Event Mixing 本底进行扣除
    //    Signal = Real - Event Mixing
    // ============================================================
    TH1D *hSub = (TH1D *)hReal->Clone("hSub");
    hSub->Add(hMixed, -1.0);

    hReal->SetLineColor(kBlue); hReal->SetLineWidth(1);
    hTrueBG->SetLineColor(kRed); hTrueBG->SetLineWidth(1);
    hMixed->SetLineColor(kGreen + 2); hMixed->SetLineWidth(1);
    hSub->SetLineColor(kBlue); hSub->SetLineWidth(1);

    TCanvas *c1 = new TCanvas("c1", "Event Mixing", 900, 650);
    hReal->SetMaximum(hReal->GetMaximum() * 1.15);
    hReal->Draw("hist");
    hTrueBG->Draw("hist same");
    hMixed->Draw("hist same");

    TLegend *leg1 = new TLegend(0.57, 0.70, 0.88, 0.88);
    leg1->AddEntry(hReal, "Real: Signal + Background", "l");
    leg1->AddEntry(hTrueBG, "True background", "l");
    leg1->AddEntry(hMixed, "Event Mixing background", "l");
    leg1->Draw();
    c1->SaveAs("EventMixingExToy_1.pdf");

    TCanvas *c2 = new TCanvas("c2", "Background Subtracted", 900, 650);
    hSub->SetMaximum(hSub->GetMaximum() * 1.15);
    hSub->Draw("hist");

    TLegend *leg2 = new TLegend(0.60, 0.78, 0.88, 0.88);
    leg2->AddEntry(hSub, "Real - Event Mixing", "l");
    leg2->Draw();
    c2->SaveAs("EventMixingExSubtracted_1.pdf");
}