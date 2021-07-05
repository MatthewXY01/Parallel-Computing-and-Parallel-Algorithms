# Parallel Computing and Parallel Algorithms

Assignments &amp; Project of CS7344:  Parallel Computing and Parallel Algorithms [2021Spring]

## Assignments

4 assignments in total, including some exercises of [《Parallel programming in C with MPI and OpenMP》](https://cs.ipb.ac.id/~auriza/parallel/book/Quinn.%202003.%20Parallel%20Programming%20in%20C%20with%20MPI%20and%20OpenMP.pdf)

**HW1:** 2.5 2.11 2.14 2.18

**HW2:** 3.17 3.19 4.2 4.8 4.11

**HW3:** 5.9 6.10 6.13 7.11

**HW4:** 8.6 8.12 9.7 9.10

## Project

### Project 1: Have a fun with parallel programming with MPI

1. **MPI_ALLGATHER:** Use *MPI_SEND* and *MPI_RECV* to implement the *MPI_ALLGATHER* function, and **compare** the performance of your implementation and the original MPI implementation.
2. **Gemm:** Please parallelize the program  in the tile unit.
   - Initialize two random 1024 *1024 matrices, and implement the matrix multiplication function.
   - Using a 4 * 4 kernel to do the pooling operation for the matrix (max or average, both ok).
   - Using a 4 * 4 kernel to do the convolution operation for the matrix.
3. **Wordcout:**
   - One folder contains 100 small files and one folder which contains 1 big file.
   - Implement the wordcount algorithm respectively for the two situations and print the results to the screen.

**Analysis: Benchmark your program with different numbers of processors and host nodes. Give an analysis on your program's output.**

### Project 2: Have a fun with parallel programming with OPENMP

1. **Monte Carlo algorithm:** Using *OpenMP* to implement the Monte Carlo algorithm.
2. **Quick Sort:** Using *OpenMP* to implement a quick sorting algorithm with large data volume which contains **1000000** number.
3. **PageRank:** Initialize a graph has **1, 024, 000** nodes and the edge count of different nodes ranges from **1 to 10**. Implement the PageRank algorithm and run for **100 iterations**.

**Analysis: Benchmark your program with different numbers of processors and host nodes. Give an analysis on your program's output.**

### Project 3: Big Data Analysis in Hadoop System

Data is available on http://ram-n.github.io/weatherData/. In this repo, I have converted the relevant data to CSV format.

You need use your weatherdata program to get the statistics of the highest and lowest temperature for all the files in the data directory, respectively.

Each project folder contains a `run.sh` that runs all test programs.

