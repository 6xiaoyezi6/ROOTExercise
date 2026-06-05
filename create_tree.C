void create_tree(){

  // 1. 常量声明
  const Int_t nEvents = 500000;

  const Double_t D = 5.0;
  const Double_t L = 1.0;
  const Double_t T = 0.05;

  const Double_t Lambda = 3.8;
  const Double_t Vsc = 0.075;

  const Double_t a = 0.83;
  const Double_t b = 2.82;
  const Double_t c = 0.25;
  const Double_t d = 0.93;

  const Double_t En0 = 100.0;
  const Double_t EnFWHM = 50.0;
  const Double_t sigmaEn = EnFWHM / 2.35;

  const Double_t Egmean = 15.0;
  const Double_t RatioGamma = 0.3;

  const Double_t sigt_floor = 0.5;
  const Double_t sigt_no = 5.0;
  const Double_t sigt_stat = 2.0;

  const Double_t Walk_AL = 40.0;
  const Double_t Walk_AR = 45.0;

  const Double_t gL_qe = 10;
  const Double_t gR_qe = 15;

  const Double_t Rq = 0.1;

  const Double_t t0L = 5.5;
  const Double_t t0R = 20.4;

  const Double_t A_th = 5;

  // 2. Tree变量
  Double_t x_true, e_true, tof_true;
  Double_t tL_true, tR_true, tL, tR;
  Double_t qL, qR, AL, AR;
  Int_t pid;

  // 3. 文件与Tree
  TFile *f = new TFile("tree.root", "recreate");
  TTree *t = new TTree("tree", "Simulated Data");

  t->Branch("x_true", &x_true, "x_true/D");
  t->Branch("e_true", &e_true, "e_true/D");
  t->Branch("tof_true", &tof_true, "tof_true/D");
  t->Branch("pid", &pid, "pid/I");

  t->Branch("tL_true", &tL_true, "tL_true/D");
  t->Branch("tR_true", &tR_true, "tR_true/D");

  t->Branch("tL", &tL, "tL/D");
  t->Branch("tR", &tR, "tR/D");

  t->Branch("AL", &AL, "AL/D");
  t->Branch("AR", &AR, "AR/D");

  TH1D *htof = new TH1D("htof", "Reconstructed TOF", 1000, 0, 100);

  TRandom3 *gr = new TRandom3(0);

  std::cout << "Starting Simulation..." << std::endl;

  // 4. 事件循环
  for(int i = 0; i < nEvents; i++){

    if(i % (nEvents/20) == 0){
      Double_t progress = (Double_t)i / nEvents * 100.0;
      std::cout << "\rProcessing: " << (int)progress << "% completed..." << std::flush;
    }

    // 真值
    x_true = gr->Uniform(-L, L);

    Double_t y_prime = gr->Uniform(-T/2.0, T/2.0);

    Double_t distance = TMath::Sqrt(pow(D + y_prime, 2) + pow(x_true, 2));

    Double_t E_dep = 0;
    Double_t Eee = 0;

    // Gamma
    if(gr->Uniform() < RatioGamma){

      pid = 0;

      e_true = gr->Exp(Egmean);

      tof_true = 3.333 * distance;

      E_dep = e_true;

      Eee = E_dep;
    }

    // Neutron
    else{

      pid = 1;

      e_true = gr->Gaus(En0, sigmaEn);

      if(e_true <= 0) continue;

      tof_true = 72.3 / TMath::Sqrt(e_true) * distance;

      Double_t Ep = e_true * gr->Uniform(0.0, 1.0);

      Eee = a * Ep - b * (1.0 - TMath::Exp(-c * pow(Ep, d)));

      if(Eee <= 0) continue;

      E_dep = Eee;
    }

    // 能量响应
    Double_t sigmaE = 0.1 * E_dep;

    Double_t Q = gr->Gaus(E_dep, sigmaE);

    if(Q <= 0) continue;

    qL = (Q / 2.0) * TMath::Exp(-(L + x_true) / Lambda);
    qR = (Q / 2.0) * TMath::Exp(-(L - x_true) / Lambda);

    qL = gr->Gaus(qL, qL * Rq / 2.35);
    qR = gr->Gaus(qR, qR * Rq / 2.35);

    AL = gL_qe * qL;
    AR = gR_qe * qR;

    // Trigger
    if(AL < A_th || AR < A_th) continue;

    // 真时间
    tL_true = tof_true + (L + x_true) / Vsc + t0L;
    tR_true = tof_true + (L - x_true) / Vsc + t0R;

    // 时间分辨
    Double_t sigma_tL = TMath::Sqrt(sigt_floor * sigt_floor + pow(sigt_stat / TMath::Sqrt(AL), 2) + sigt_no * sigt_no);

    Double_t sigma_tR = TMath::Sqrt(sigt_floor * sigt_floor + pow(sigt_stat / TMath::Sqrt(AR), 2) + sigt_no * sigt_no);

    // Time-walk
    tL = gr->Gaus(tL_true, sigma_tL) + Walk_AL / TMath::Sqrt(AL);

    tR = gr->Gaus(tR_true, sigma_tR) + Walk_AR / TMath::Sqrt(AR);

    // TOF重建
    Double_t tof_rec = (tL + tR) / 2.0;

    htof->Fill(tof_rec);

    t->Fill();
  }

  std::cout << "\rProcessing: 100% completed!" << std::endl;

  htof->Write();
  t->Write();

  f->Close();

  std::cout << "Data saved to tree.root" << std::endl;
}