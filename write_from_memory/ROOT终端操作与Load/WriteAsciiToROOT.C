#include "TAsciiFile.h"
#include "TFile.h"

void WriteAsciiToROOT(const char *fileName = "Espri.root") {
  TFile *file = TFile::Open(fileName, "UPDATE");
  TAsciiFile text;
  text.SetName("TestAscii");
  text.SetTitle("Test text written by WriteAsciiToROOT.C");
  // 下面的内容是写入的
  text.AppendLine("# NPTool TAsciiFile test");
  text.AppendLine("Message = Hello from WriteAsciiToROOT.C");
  text.AppendLine("Value = 123.456789");

  file->cd();
  Int_t bytes = text.Write("TestAscii", TObject::kOverwrite);
  file->Close();
  delete file;
}
