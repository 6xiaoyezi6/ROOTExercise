// EventMixingExToy_3.C
// 模拟静止的 10Be*(10.15 MeV) -> 6He + alpha 两体衰变
// 真实信号采用 Breit-Wigner 分布，Gamma = 0.296 MeV
// 真实“本底”中 6He 和 alpha 的四动量彼此独立连续采样
// 本例不使用 MC truth 选择 Event Mixing 事件，而是使用可观测量 multiplicity 选择事件池
// 目的是演示：合理选择 event pool 可以降低 signal 对 Mixed Event 本底形状的污染

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

void EventMixingExToy_3()
{
    gStyle->SetOptStat(0);

    const int N = 200000;
    const double signalFraction = 0.30;
    TRandom3 rng(42);

    // 这里只保存满足事件池条件的事件
    // 注意：程序在选择事件池时不使用 isSignal
    std::vector<TLorentzVector> he6Pool, alphaPool;
    he6Pool.reserve(N); alphaPool.reserve(N);

    TH1D *hReal = new TH1D("hReal", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hTrueBG = new TH1D("hTrueBG", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hMixed = new TH1D("hMixed", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);

    hReal->Sumw2(); hTrueBG->Sumw2(); hMixed->Sumw2();

    int poolSignal = 0;
    int poolBackground = 0;

    // ============================================================
    // 1. 构造真实数据
    //    Real = Signal + True Background
    // ============================================================
    for (int i = 0; i < N; ++i) {
        bool isSignal = rng.Uniform() < signalFraction;
        TLorentzVector he6, alpha;
        int multiplicity = 0;

        // --------------------------------------------------------
        // 1.1 真实信号事件
        // --------------------------------------------------------
        if (isSignal) {
            double Ex = SampleResonanceEx(rng);
            double p = TwoBodyMomentum(Ex);
            TVector3 p6 = RandomMomentum(rng, p);
            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(-p6, MA);

            // 假设 signal 大多数是低 multiplicity 事件
            multiplicity = (rng.Uniform() < 0.90) ? 2 : 3;
        }

        // --------------------------------------------------------
        // 1.2 真实“本底”事件
        // --------------------------------------------------------
        else {
            double p6mag = SampleBackgroundMomentum(rng);
            double pAmag = SampleBackgroundMomentum(rng);
            TVector3 p6 = RandomMomentum(rng, p6mag);
            TVector3 pA = RandomMomentum(rng, pAmag);
            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(pA, MA);

            // 假设 background 大多数具有较高 multiplicity
            double r = rng.Uniform();
            if (r < 0.10) multiplicity = 2;
            else if (r < 0.70) multiplicity = 3;
            else multiplicity = 4;
        }

        double ExReco = ReconstructEx(he6, alpha);
        hReal->Fill(ExReco);
        if (!isSignal) hTrueBG->Fill(ExReco);

        // ========================================================
        // 2. 选择 Event Mixing 的事件池
        //    实际选择只使用可观测量 multiplicity
        //    不使用 isSignal 判断
        //    multiplicity >= 3 的事件进入事件池
        // ========================================================
        if (multiplicity >= 3) {
            he6Pool.push_back(he6);
            alphaPool.push_back(alpha);

            // 下面两个计数只用于 toy MC 检查事件池纯度
            if (isSignal) poolSignal++;
            else poolBackground++;
        }
    }

    // ============================================================
    // 3. Event Mixing
    //    使用筛选后的事件池：
    //    (6He_i, alpha_(i+1))
    // ============================================================
    int NPool = he6Pool.size();

    for (int i = 0; i < NPool; ++i) {
        int j = (i + 1) % NPool;
        double ExMixed = ReconstructEx(he6Pool[i], alphaPool[j]);
        hMixed->Fill(ExMixed);
    }

    // ============================================================
    // 4. Event Mixing 本底归一化
    // ============================================================
    int b1 = hReal->FindBin(7.42);
    int b2 = hReal->FindBin(8.50 - 1e-6);

    double realSideband = hReal->Integral(b1, b2);
    double mixedSideband = hMixed->Integral(b1, b2);
    double alphaNorm = realSideband / mixedSideband;

    hMixed->Scale(alphaNorm);

    double poolSignalFraction = static_cast<double>(poolSignal) / NPool;

    std::cout << "Total events = " << N << std::endl;
    std::cout << "Event pool size = " << NPool << std::endl;
    std::cout << "Signal events in pool = " << poolSignal << std::endl;
    std::cout << "Background events in pool = " << poolBackground << std::endl;
    std::cout << "Signal fraction in pool = " << poolSignalFraction << std::endl;
    std::cout << "Normalization alpha = " << alphaNorm << std::endl;

    // ============================================================
    // 5. 扣除 Event Mixing 本底
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
    leg1->AddEntry(hMixed, "Selected-pool Event Mixing", "l");
    leg1->Draw();
    c1->SaveAs("EventMixingExToy_3.pdf");

    TCanvas *c2 = new TCanvas("c2", "Background Subtracted", 900, 650);
    hSub->SetMaximum(hSub->GetMaximum() * 1.15);
    hSub->Draw("hist");

    TLegend *leg2 = new TLegend(0.60, 0.78, 0.88, 0.88);
    leg2->AddEntry(hSub, "Real - Event Mixing", "l");
    leg2->Draw();
    c2->SaveAs("EventMixingExSubtracted_3.pdf");
}