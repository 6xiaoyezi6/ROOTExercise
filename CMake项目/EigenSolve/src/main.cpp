#include "MatrixSolver.h"

#include <exception>
#include <iostream>

int main() {
    try {
        const MatrixSolver matrix;
        const EigenSolver eigenSolver;
        const ROOTSolver rootSolver;

        matrix.PrintComparison(eigenSolver.Solve(matrix),
                               rootSolver.Solve(matrix));
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
