#include "MatrixSolver.h"

#include <Eigen/Dense>
#include <TDecompLU.h>
#include <TMath.h>
#include <TMatrixD.h>
#include <TVectorD.h>

#include <iostream>
#include <stdexcept>

void MatrixSolver::PrintInput() const {
    std::cout << "Matrix A:\n";
    for (std::size_t row = 0; row < A_.size(); ++row) {
        for (std::size_t column = 0; column < A_[row].size(); ++column) {
            std::cout << A_[row][column] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "\nVector b:\n";
    for (std::size_t row = 0; row < b_.size(); ++row) {
        std::cout << b_[row] << '\n';
    }
}

void MatrixSolver::PrintComparison(const std::vector<double>& eigenX,
                                   const std::vector<double>& rootX) const {
    std::cout << "\nThe following is output\n";
    PrintInput();

    std::cout << "\nSolution x calculated by Eigen:\n";
    for (std::size_t row = 0; row < eigenX.size(); ++row) {
        std::cout << eigenX[row] << '\n';
    }

    std::cout << "\nSolution x calculated by ROOT TDecompLU:\n";
    for (std::size_t row = 0; row < rootX.size(); ++row) {
        std::cout << rootX[row] << '\n';
    }

    double maximumDifference = 0.0;
    for (std::size_t row = 0; row < eigenX.size(); ++row) {
        const double difference = TMath::Abs(eigenX[row] - rootX[row]);
        if (difference > maximumDifference) {
            maximumDifference = difference;
        }
    }

    std::cout << "\nMaximum absolute difference: "
              << maximumDifference << '\n';
}

std::vector<double> EigenSolver::Solve(const MatrixSolver& matrix) const {
    const std::vector<std::vector<double>>& A = matrix.GetA();
    const std::vector<double>& b = matrix.GetB();
    const Eigen::Index n = static_cast<Eigen::Index>(A.size());
    Eigen::MatrixXd eigenA(n, n);
    Eigen::VectorXd eigenB(n);

    for (Eigen::Index row = 0; row < n; ++row) {
        for (Eigen::Index column = 0; column < n; ++column) {
            eigenA(row, column) = A[row][column];
        }
        eigenB(row) = b[row];
    }

    Eigen::FullPivLU<Eigen::MatrixXd> decomposition(eigenA);
    if (!decomposition.isInvertible()) {
        throw std::runtime_error(
            "Eigen could not find a unique solution.");
    }

    const Eigen::VectorXd eigenX = decomposition.solve(eigenB);
    std::vector<double> result(n);

    for (Eigen::Index row = 0; row < n; ++row) {
        result[row] = eigenX(row);
    }

    return result;
}

std::vector<double> ROOTSolver::Solve(const MatrixSolver& matrix) const {
    const std::vector<std::vector<double>>& A = matrix.GetA();
    const std::vector<double>& b = matrix.GetB();
    const Int_t n = static_cast<Int_t>(A.size());
    TMatrixD rootA(n, n);
    TVectorD rootB(n);

    for (Int_t row = 0; row < n; ++row) {
        for (Int_t column = 0; column < n; ++column) {
            rootA(row, column) = A[row][column];
        }
        rootB(row) = b[row];
    }

    TDecompLU decomposition(rootA);
    Bool_t solved = kFALSE;
    const TVectorD rootX = decomposition.Solve(rootB, solved);

    if (!solved) {
        throw std::runtime_error(
            "ROOT TDecompLU could not find a unique solution.");
    }

    std::vector<double> result(n);
    for (Int_t row = 0; row < n; ++row) {
        result[row] = rootX(row);
    }

    return result;
}
