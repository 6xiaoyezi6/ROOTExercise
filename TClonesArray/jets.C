/*
- write_events()：生成 JetEvent.root，写入 TTree
- read_events()：读回 JetEvent.root，打印第一个 event
- pileup_events()：保持原来的 pileup 读取测试
- tree130_jets()：作为入口，加载本地 JetEvent.cxx+，然后依次调用上面三个函数
调用自己编写的JetEvent类
*/   

#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TSystem.h"
#include "JetEvent.h"
#include "Riostream.h"

void write_events(Int_t nev = 100)
{
   TFile *file = new TFile("JetEvent.root", "RECREATE");
   TTree *tree = new TTree("T", "Event example with Jets");
   JetEvent *event = new JetEvent();

   tree->Branch("event", "JetEvent", &event, 8000, 2);

   for (Int_t ev = 0; ev < nev; ev++) {
      event->Build();
      tree->Fill();
   }

   tree->Print();
   tree->Write();
   file->Close();

   delete event;
   delete file;
}

void read_events()
{
   TFile *file = new TFile("JetEvent.root", "READ");
   TTree *tree = (TTree *)file->Get("T");

   JetEvent *event = 0;
   tree->SetBranchAddress("event", &event);

   Long64_t nentries = tree->GetEntries();
   for (Long64_t ev = 0; ev < nentries; ev++) 
   {
      tree->GetEntry(ev);

      std::cout << " Event: " << ev
                << "  Jets: " << event->GetNjet()
                << "  Tracks: " << event->GetNtrack()
                << "  Hits A: " << event->GetNhitA()
                << "  Hits B: " << event->GetNhitB()
                << std::endl;

      break; // print the first event only
   }

   file->Close();
   delete file;
}

void pileup_events(Int_t nev = 200)
{
   TFile *file = new TFile("JetEvent.root", "READ");
   TTree *tree = (TTree *)file->Get("T");
 

   const Int_t loopMax = 10;
   JetEvent *events[loopMax];

   for (Int_t loop = 0; loop < loopMax; loop++) {
      events[loop] = 0;
   }

   for (Long64_t ev = 0; ev < nev; ev++) {
      if (ev % 10 == 0) {
         std::cout << "building pileup: " << ev << std::endl;
      }

      for (Int_t loop = 0; loop < loopMax; loop++) {
         Int_t entry = gRandom->Uniform(loopMax);
         tree->SetBranchAddress("event", &events[loop]);
         tree->GetEntry(entry);
      }
   }

   file->Close();
   delete file;
}

void jets(Int_t nev = 100, Int_t npileup = 200)
{
   gROOT->LoadMacro("JetEvent.cxx+");

   write_events(nev);
   read_events();
   pileup_events(npileup);
}
