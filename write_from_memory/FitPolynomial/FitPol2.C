void FitPol2()
{
 TCanvas * c = new TCanvas("c","c",1000,800);
 TRandom r(1234);
 TVector3 v;
 TGraph*g2 = new TGraph();
 TF1*fquadra=new TF1("fquadra","[0]*x*x+[1]*x+[2]",0,5);
 fquadra->SetParameters(1,2,3);
 for(int i=0;i<5;i++)
    {
    double x = i;
    double y = fquadra->Eval(i)+r.Gaus(0,1);
    double z = fquadra->Eval(i);

    v.SetXYZ(x,y,z);
    g2->SetPoint(i,v.X(),v.Y());

    cout<<"y="<<y<<endl;
    cout<<"z="<<z<<endl;
        
    }
 g2->SetMarkerStyle(21);
 g2->Draw("AP");

// g2->Fit(fquadra);
}

