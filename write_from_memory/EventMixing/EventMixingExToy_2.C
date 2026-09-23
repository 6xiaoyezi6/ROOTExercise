// EventMixingExToy.C
// 模拟静止的 10Be*(10.15 MeV) -> 6He + alpha 两体衰变
// 真实信号采用 Breit-Wigner 分布，中心 10.15 MeV，Gamma = 0.296 MeV
// 真实“本底”中 6He 和 alpha 的动量大小、方向均彼此独立连续采样
// 本例利用 MC truth 单独保存真实本底事件，并只在真实本底事件池中进行 Event Mixing
// 比较真实本底与 Event Mixing 本底，检验 Event Mixing 是否能够恢复非关联本底

//这里只使用真实本底作为Event Mixing的"事件池"
//Event Mixing产生的事件与"真实本底"一样

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

void EventMixingExToy_2()
{
    gStyle->SetOptStat(0);

    const int N = 200000;
    const double signalFraction = 0.30;
    TRandom3 rng(42);

    // 这里只保存真实“本底”事件中的单粒子四动量
    std::vector<TLorentzVector> he6BGList, alphaBGList;
    he6BGList.reserve(N); alphaBGList.reserve(N);

    TH1D *hReal = new TH1D("hReal", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hTrueBG = new TH1D("hTrueBG", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);
    TH1D *hMixed = new TH1D("hMixed", ";E_{x} (MeV);Counts / 0.02 MeV", 380, 7.4, 15.0);

    hReal->Sumw2(); hTrueBG->Sumw2(); hMixed->Sumw2();

    // ============================================================
    // 1. 构造“真实数据”
    //    Real = Signal + True Background
    // ============================================================
    for (int i = 0; i < N; ++i) {
        bool isSignal = rng.Uniform() < signalFraction;
        TLorentzVector he6, alpha;

        // ========================================================
        // 1.1 构造真实信号事件
        //     10Be*(10.15 MeV) -> 6He + alpha
        //     两个粒子来自同一次两体衰变，因此严格相关
        //     p_6He = -p_alpha
        // ========================================================
        if (isSignal) {
            double Ex = SampleResonanceEx(rng);
            double p = TwoBodyMomentum(Ex);
            TVector3 p6 = RandomMomentum(rng, p);

            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(-p6, MA);
        }

        // ========================================================
        // 1.2 构造真实“本底”事件
        //     6He 和 alpha 不来自共同的两体衰变
        //     动量大小独立采样，方向也独立采样
        // ========================================================
        else {
            double p6mag = SampleBackgroundMomentum(rng);
            double pAmag = SampleBackgroundMomentum(rng);

            TVector3 p6 = RandomMomentum(rng, p6mag);
            TVector3 pA = RandomMomentum(rng, pAmag);

            he6 = MakeParticle(p6, M6);
            alpha = MakeParticle(pA, MA);

            // MC truth：把真正的本底事件单独保存
            he6BGList.push_back(he6);
            alphaBGList.push_back(alpha);
        }

        double ExReco = ReconstructEx(he6, alpha);

        // 蓝色：真实数据 = Signal + Background
        hReal->Fill(ExReco);

        // 红色：MC truth 中真正的本底
        if (!isSignal) hTrueBG->Fill(ExReco);
    }

    // ============================================================
    // 2. 使用 Event Mixing 构造本底
    //
    //    这里只使用真实“本底”事件池
    //
    //    True Background:
    //        (6He_i, alpha_i)
    //
    //    Event Mixing:
    //        (6He_i, alpha_(i+1))
    //
    //    因为本底中的 6He 和 alpha 本来就是独立的，
    //    所以 Mixed Event 应该能够恢复 True Background
    // ============================================================
    int NBG = he6BGList.size();

    for (int i = 0; i < NBG; ++i) {
        int j = (i + 1) % NBG;
        double ExMixed = ReconstructEx(he6BGList[i], alphaBGList[j]);
        hMixed->Fill(ExMixed);
    }

    // ============================================================
    // 3. Mixed Event 归一化
    //
    //    为了单纯检验 Event Mixing 是否能恢复真实本底，
    //    这里利用 MC truth，在 7.42-8.50 MeV 区域让
    //    Mixed Event 与 True Background 归一化
    //
    //    注意：真实实验中并不知道 hTrueBG
    // ============================================================
    int b1 = hTrueBG->FindBin(7.42);
    int b2 = hTrueBG->FindBin(8.50 - 1e-6);

    double trueBGSideband = hTrueBG->Integral(b1, b2);
    double mixedSideband = hMixed->Integral(b1, b2);
    double alphaNorm = trueBGSideband / mixedSideband;

    hMixed->Scale(alphaNorm);

    std::cout << "6He + alpha threshold = " << Threshold << " MeV" << std::endl;
    std::cout << "Total events = " << N << std::endl;
    std::cout << "True background events = " << NBG << std::endl;
    std::cout << "Normalization alpha = " << alphaNorm << std::endl;

    // ============================================================
    // 4. 使用 Event Mixing 本底进行扣除
    //    Signal = Real - Mixed Background
    // ============================================================
    TH1D *hSub = (TH1D *)hReal->Clone("hSub");
    hSub->Add(hMixed, -1.0);

    hReal->SetLineColor(kBlue); hReal->SetLineWidth(1);
    hTrueBG->SetLineColor(kRed); hTrueBG->SetLineWidth(1);
    hMixed->SetLineColor(kGreen + 2); hMixed->SetLineWidth(1);
    hSub->SetLineColor(kBlue); hSub->SetLineWidth(1);

    // ============================================================
    // Figure 1：真实数据、真实本底、Event Mixing 本底
    // ============================================================
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

    c1->SaveAs("EventMixingExToy_2.pdf");

    // ============================================================
    // Figure 2：扣除 Event Mixing 本底后的激发能谱
    // ============================================================
    TCanvas *c2 = new TCanvas("c2", "Background Subtracted", 900, 650);

    hSub->SetMaximum(hSub->GetMaximum() * 1.15);
    hSub->Draw("hist");

    TLegend *leg2 = new TLegend(0.60, 0.78, 0.88, 0.88);
    leg2->AddEntry(hSub, "Real - Event Mixing", "l");
    leg2->Draw();

    c2->SaveAs("EventMixingExSubtracted_2.pdf");
}