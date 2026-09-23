// 本程序使用 ROOT 构造一个最简单的 Event Mixing 方法示例
// 模拟数据由真实关联事件和随机非关联本底两部分组成
// 真实关联事件中两个粒子的方位角差集中在 180 度附近
// 通过将不同事件中的粒子重新配对构造 Mixed Event 本底分布
// 对 Mixed Event 进行侧带归一化后，从真实事件谱中扣除组合本底

#include <TCanvas.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TMath.h>
#include <TLegend.h>
#include <TStyle.h>
#include <vector>
#include <iostream>

double DeltaPhi(double phi1, double phi2)
{
    double dphi = (phi1 - phi2) * 180.0 / TMath::Pi();
    while (dphi < 0.0) dphi += 360.0;
    while (dphi >= 360.0) dphi -= 360.0;
    return dphi;
}

void EventMixingToy()
{
    gStyle->SetOptStat(0);

    const int N = 200000;
    const double signalFraction = 0.30;
    const double sigmaPhi = 8.0 * TMath::Pi() / 180.0;

    TRandom3 rng(42);
    std::vector<double> phiA, phiB;
    phiA.reserve(N); phiB.reserve(N);

    TH1D *hReal = new TH1D("hReal", ";#Delta#phi (deg);Counts / 2 deg", 180, 0, 360);
    TH1D *hTrueBG = new TH1D("hTrueBG", ";#Delta#phi (deg);Counts / 2 deg", 180, 0, 360);
    TH1D *hMixed = new TH1D("hMixed", ";#Delta#phi (deg);Counts / 2 deg", 180, 0, 360);

    // Generate real events
    for (int i = 0; i < N; ++i) {
        double a = rng.Uniform(0, 2.0 * TMath::Pi());
        double b = 0.0;
        bool isSignal = rng.Uniform() < signalFraction;

        if (isSignal) b = a + TMath::Pi() + rng.Gaus(0, sigmaPhi);//真事件粒子ab的phi角差是180度
        else b = rng.Uniform(0, 2.0 * TMath::Pi());//本底a,b的phi角是随机分布的

        while (b < 0.0) b += 2.0 * TMath::Pi();
        while (b >= 2.0 * TMath::Pi()) b -= 2.0 * TMath::Pi();

        phiA.push_back(a); phiB.push_back(b);
        double dphi = DeltaPhi(a, b);
        hReal->Fill(dphi);
        if (!isSignal) hTrueBG->Fill(dphi);
    }

    //构造混合事件
    //真实事件对配对关系是(a_{i},b_{i}),混合事件的配对关系是(a_{i},b_{i+1})
    for (int i = 0; i < N; ++i) {
        int j = (i + 1) % N;
        hMixed->Fill(DeltaPhi(phiA[i], phiB[j]));
    }

    //混合事件的归一化：这是关键。Event Mixing只能给出本底分布的形状，并不能给出本底的数目
    //Sigal = H-alpha*H_{mix}
    int b1 = hReal->FindBin(0.0);
    int b2 = hReal->FindBin(140.0);
    int b3 = hReal->FindBin(220.0);
    int b4 = hReal->FindBin(360.0 - 1e-6);

    double realSideband = hReal->Integral(b1, b2) + hReal->Integral(b3, b4);
    double mixedSideband = hMixed->Integral(b1, b2) + hMixed->Integral(b3, b4);
    double alpha = realSideband / mixedSideband;//归一化系数alpha
    hMixed->Scale(alpha);

    std::cout << "Normalization alpha = " << alpha << std::endl;
    std::cout << "Expected background fraction = " << 1.0 - signalFraction << std::endl;

    // Background subtraction
    TH1D *hSub = (TH1D *)hReal->Clone("hSub");
    hSub->Add(hMixed, -1.0);
    hSub->SetTitle(";#Delta#phi (deg);Counts / 2 deg");

    // Colors
    hReal->SetLineColor(kBlue); hReal->SetLineWidth(1);
    hTrueBG->SetLineColor(kRed); hTrueBG->SetLineWidth(1);
    hMixed->SetLineColor(kGreen + 2); hMixed->SetLineWidth(1);
    hSub->SetLineColor(kBlue); hSub->SetLineWidth(1);

    // Figure 1: Real, true background and Event Mixing
    TCanvas *c1 = new TCanvas("c1", "Event Mixing", 900, 650);
    hReal->SetMaximum(hReal->GetMaximum() * 1.20);
    hReal->Draw("hist");
    hTrueBG->Draw("hist same");
    hMixed->Draw("hist same");

    TLegend *leg1 = new TLegend(0.16, 0.70, 0.50, 0.88);
    leg1->AddEntry(hReal, "Real events: Signal + Background", "l");
    leg1->AddEntry(hTrueBG, "True background", "l");
    leg1->AddEntry(hMixed, "Event Mixing background", "l");
    leg1->Draw();
    c1->SaveAs("EventMixingToy.pdf");

    // Figure 2: Spectrum after Event Mixing subtraction
    TCanvas *c2 = new TCanvas("c2", "Background Subtracted", 900, 650);
    hSub->SetMaximum(hSub->GetMaximum() * 1.20);
    hSub->Draw("hist");

    TLegend *leg2 = new TLegend(0.16, 0.78, 0.48, 0.88);
    leg2->AddEntry(hSub, "Real - Event Mixing", "l");
    leg2->Draw();
    c2->SaveAs("EventMixingSubtracted.pdf");
}