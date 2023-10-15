# tt-open-wbo-inc
TT-Open-WBO-Inc: an anytime MaxSAT solver by Alexander Nadel availalbe from 2019, where the GitHub version is initialized with the version submitted to MaxSAT Evaluation 2023.

When using TT-Open-WBO-Inc, please cite the following paper:

"Polarity and Variable Selection Heuristics for SAT-Based Anytime MaxSAT", Alexander Nadel, J. Satisf. Boolean Model. Comput. 12(1): 17-22 (2020)

The license is in code/LICENSE.

To compile, make sure that your g++ version is at least 10.1 and run:

cd code
make rs -j

To change the underlying SAT solver from IntelSAT to Glucose, edit code/Makefile as follows:

# Glucose4_1
SOLVERNAME = "Glucose4_1"
SOLVERDIR  = glucose4.1
NSPACE     = Glucose

# IntelSATSolver
#SOLVERNAME = "IntelSATSolver"
#SOLVERDIR  = topor
#NSPACE     = ToporWrapper

