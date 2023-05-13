Readme for Assignment 2 of ECE5544 Compiler Optimization
Group: Shambhavi Kuthe, Rohit Mehta

Instructions:
Copy the shambhavianil-rohitnm folder in Transforms folder in llvm-project preferably.
All the commands are given for test files that are placed next to the pass file i.e. available-test-m2r.bc, reaching-test-m2r.bc and liveness-test-m2r.bc are placed in Dataflow folder next to all the .cpp and .h files. Thus, the path for running the pass only includes ./ as the path.

Path to the .so file needs to be changed if any other location files are used.

//To run Available pass:

//To compile, build and run dataflow analysis on available.cpp with available-test-m2r.bc test file :
cd Dataflow
make
opt -enable-new-pm=0 -load ./available.so -available available-test-m2r.bc -o out

//To compile, build and run dataflow analysis on reaching.cpp with reaching-test-m2r.bc test file :
cd Dataflow
make
opt -enable-new-pm=0 -load ./reaching.so -reaching reaching-test-m2r.bc -o out

//To compile, build and run dataflow analysis on liveness.cpp with liveness-test-m2r.bc test file :
cd Dataflow
make
opt -enable-new-pm=0 -load ./liveness.so -liveness liveness-test-m2r.bc -o out




