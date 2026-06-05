void read_tvector3()
{
    TFile *f = new TFile("test_tvector3.root", "READ");
    TTree *t = (TTree*)f->Get("tree");

    TVector3 *pos = nullptr;  //这就是指针，要和make_tvector3.C对应上
    TVector3 *vertex = nullptr;

    t->SetBranchAddress("pos", &pos);
    t->SetBranchAddress("vertex",&vertex);


    TH1F *hTheta = new TH1F("hTheta", "Theta of TVector3;#theta [rad];Counts", 100, 0, 3.2);
    TH2F* hvertex =new TH2F("hvertex","Vertx distribution;#x;#y",120,-30,30,120,-30,30);

    Long64_t nentries = t->GetEntriesFast();

    for (Long64_t i = 0; i < nentries; i++)
    {
        t->GetEntry(i);

        double thetaValue = pos->Theta();
        hTheta->Fill(thetaValue);

        double vertexX = vertex->X();
        double vertexY = vertex->Y();
        hvertex->Fill(vertexX,vertexY);
    }

    TCanvas* c1 = new TCanvas("c1","c1",800,600);
    c1->Divide(2,1);
    c1->cd(1);

    hTheta->Draw();
    c1->cd(2);
    hvertex->Draw();
    c1->Draw();

}