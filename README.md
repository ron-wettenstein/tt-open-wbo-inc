TT-Open-WBO-Inc: an anytime MaxSAT solver by Alexander Nadel available from 2019, where its GitHub version was initialized with the solver submitted to MaxSAT Evaluation 2023. 

When using TT-Open-WBO-Inc, please cite the following paper:

  "Polarity and Variable Selection Heuristics for SAT-Based Anytime MaxSAT", Alexander Nadel, J. Satisf. Boolean Model. Comput. 12(1): 17-22 (2020)

The underlying algorithms are further detailed in:

  "Anytime Algorithms for MaxSAT and Beyond", Alexander Nadel, FMCAD'20 (tutorial)
  
  “On Optimizing a Generic Function in SAT”, Alexander Nadel, FMCAD'20.  
  
  “Anytime Weighted MaxSAT with Improved Polarity Selection and Bit-Vector Optimization”, Alexander Nadel, FMCAD'19.
  
  “Solving MaxSAT with Bit-Vector Optimization”, Alexander Nadel, SAT'18.

The license is MIT. See code/LICENSE for further details.

To compile, make sure that your g++ version is at least 10.1 and run:

  cd code
  make rs -j

To change the underlying SAT solver from IntelSAT to Glucose, edit code/Makefile as follows:

  #Glucose4_1
  
  SOLVERNAME = "Glucose4_1"
  SOLVERDIR  = glucose4.1
  NSPACE     = Glucose
  
  #IntelSATSolver
  #SOLVERNAME = "IntelSATSolver"
  #SOLVERDIR  = topor
  #NSPACE     = ToporWrapper
  
