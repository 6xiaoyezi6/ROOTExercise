void make_tvector3()
{
    TRandom3 *r = new TRandom3(0);
    // 创建输出文件
    TFile *f = new TFile("test_tvector3.root", "RECREATE");
    TTree *t = new TTree("tree", "Simple TVector3 test tree");

    TVector3 *pos = nullptr;
    TVector3 *vertex = nullptr; //这是指针，后面调用都要用->

    //如果用的是普通对象：
    //TVector3 pos;
    //pos.SetXYZ(1, -3, 10);


    

    // 建立分支
    t->Branch("pos", &pos);
    t->Branch("vertex",&vertex);


    for (int i = 0; i < 50000; i++)
    {
        double vertexX = r->Gaus(2,6);
        double vertexY = r->Gaus(-1,4);
        double vertexZ = 1;

        vertex->SetXYZ(vertexX,vertexY,vertexZ);

        double x = i * 1.0;
        double y = i * 2.0 - 5.0;
        double z = 10.0;   

        pos->SetXYZ(x, y, z);


        t->Fill();
    }
    

    // 写入文件
    t->Write();
    f->Close();

}