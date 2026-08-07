#ifndef MATRIX_SOLVER_H
#define MATRIX_SOLVER_H

#include <iostream>
#include <stdexcept>
#include <vector>

class MatrixSolver {
public:
    MatrixSolver() {
        ReadInput();
    }

    const std::vector<std::vector<double>>& GetA() const {
        return A_;
    }

    const std::vector<double>& GetB() const {
        return b_;
    }

    void PrintInput() const;
    void PrintComparison(const std::vector<double>& eigenX,
                         const std::vector<double>& rootX) const;

private:
    void ReadInput() {
        int n = 0;

        std::cout << "Enter the dimension n of matrix A: ";
        if (!(std::cin >> n) || n <= 0) {
            throw std::runtime_error("n must be a positive integer.");
        }

        A_.assign(n, std::vector<double>(n));
        b_.assign(n, 0.0);

        std::cout << "Enter " << n * n<< " elements of matrix A row by row:\n";
        for (int row = 0; row < n; ++row) {
            for (int column = 0; column < n; ++column) {
                if (!(std::cin >> A_[row][column])) {
                    throw std::runtime_error("invalid element in matrix A.");
                }
            }
        }

        std::cout << "Enter " << n << " elements of vector b:\n";
        for (int row = 0; row < n; ++row) {
            if (!(std::cin >> b_[row])) {
                throw std::runtime_error("invalid element in vector b.");
            }
        }
    }

    std::vector<std::vector<double>> A_;
    std::vector<double> b_;
};

class EigenSolver {
public:
    std::vector<double> Solve(const MatrixSolver& matrix) const;
};

class ROOTSolver {
public:
    std::vector<double> Solve(const MatrixSolver& matrix) const;
};

#endif
