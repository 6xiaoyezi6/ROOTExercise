#include "TMath.h"

Double_t Gaussfunction(Double_t *x, Double_t *par) {
    Double_t A = par[0];
    Double_t mu = par[1];
    Double_t sigma = par[2];

    return A * TMath::Exp(-0.5 * TMath::Power((x[0] - mu) / sigma, 2));
}