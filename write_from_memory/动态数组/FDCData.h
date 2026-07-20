//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jul 20 19:21:02 2026 by ROOT version 6.40.02
// from TTree tree/Simulated FDC hit data
// found on file: FDCData.root
//////////////////////////////////////////////////////////

#ifndef FDCData_h
#define FDCData_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"

class FDCData {
public :
   TTree          *fChain;   ///<!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; ///<!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           eventID;
   vector<int>     *FDC0_layerid;
   vector<int>     *FDC0_wireid;
   vector<double>  *FDC0_TDC;
   vector<int>     *FDC0_layer_nhits;

   // List of branches
   TBranch        *b_eventID;   ///<!
   TBranch        *b_FDC0_layerid;   ///<!
   TBranch        *b_FDC0_wireid;   ///<!
   TBranch        *b_FDC0_TDC;   ///<!
   TBranch        *b_FDC0_layer_nhits;   ///<!

   FDCData(TTree *tree=0);
   virtual ~FDCData();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef FDCData_cxx
FDCData::FDCData(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("FDCData.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("FDCData.root");
      }
      f->GetObject("tree",tree);

   }
   Init(tree);
}

FDCData::~FDCData()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t FDCData::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t FDCData::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void FDCData::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.

   // Set object pointer
   FDC0_layerid = 0;
   FDC0_wireid = 0;
   FDC0_TDC = 0;
   FDC0_layer_nhits = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("eventID", &eventID, &b_eventID);
   fChain->SetBranchAddress("FDC0_layerid", &FDC0_layerid, &b_FDC0_layerid);
   fChain->SetBranchAddress("FDC0_wireid", &FDC0_wireid, &b_FDC0_wireid);
   fChain->SetBranchAddress("FDC0_TDC", &FDC0_TDC, &b_FDC0_TDC);
   fChain->SetBranchAddress("FDC0_layer_nhits", &FDC0_layer_nhits, &b_FDC0_layer_nhits);
   Notify();
}

bool FDCData::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be for a new TTree in a TChain. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void FDCData::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t FDCData::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef FDCData_cxx
