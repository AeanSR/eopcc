# eopcc
Artifacts of the TC paper "Breaking the Interaction Wall: A DLPU-Centric Deep Learning Computing System"

### prerequisite

GCC >= 9.1.0

### build

Build EOP-C Compiler:

    g++ parser.cpp -O3 -o eopcc -std=c++17
    
Build CPULESS Simulator:

    g++ sim.cpp -O3 -o eopsim -std=c++17
    
### run

Compile and run full benchset:

    python3 run.py
    
`run.py` assumes both `eopcc` and `eopsim` binaries are installed. You may specify the directory in `$PATH`, or copy them under `/usr/bin`.

`run.py` stores the compiled bench programs under `/tmp/bench`. It may complain the directory is not found at the first time, you may create the directory manually.
