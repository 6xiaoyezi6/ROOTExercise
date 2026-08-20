#include "TCanvas.h"
#include "TLine.h"
#include "TLatex.h"

void ColorTest() {
    TCanvas *c = new TCanvas("c", "ROOT Blue Colors", 800, 500);
    c->Range(0, 0, 10, 6);

    int colors[5] = {kAzure - 4, kAzure - 2, kAzure, kAzure + 2, kAzure + 4};
    const char *names[5] = {"kAzure-4", "kAzure-2", "kAzure", "kAzure+2", "kAzure+4"};

    for (int i = 0; i < 5; i++) {
        double y = 5 - i;

        TLine *line = new TLine(2, y, 8, y);
        line->SetLineColor(colors[i]);
        line->SetLineWidth(6);
        line->Draw();

        TLatex *text = new TLatex(0.5, y, names[i]);
        text->SetTextSize(0.04);
        text->Draw();
    }

    c->SaveAs("BlueColorTest.pdf");
}