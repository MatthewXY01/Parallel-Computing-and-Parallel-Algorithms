echo -e "############### Project 1 ###############"
echo -e "# Project 1: MPI_Allgather vs. MY_Allgather\n\n"
mpicxx -o allgather_benchmark myallgather.c allgather_benchmark.c
# exec "mpicxx" -o "allgather_benchmark" "myallgather.c" "allgather_benchmark.c"
mpirun -np 4 ./allgather_benchmark MPI_Allgather 100000
mpirun -np 4 ./allgather_benchmark MY_Allgather 100000

echo -e ""
echo -e "# Project 1: GEMM"
echo -e "# Matrix Multiplication"
mpicxx -o matMulti matMulti.c
echo -e "Correctness validation: "
mpirun -np 4 ./matMulti 4 "true"
echo -e "1024x1024 4 processors: "
mpirun -np 4 ./matMulti 1024 "false"

echo -e "# Convolution Operation"
mpicxx -o convOP convOP.c
echo -e "Correctness validation: "
mpirun -np 4 ./convOP 3 2 "true"
echo -e "1024x1024 matrix 4x4 kernel 4 processors: "
mpirun -np 4 ./convOP 1024 4 "false"

echo -e "# Maxpooling Operation"
mpicxx -o poolingOP poolingOP.c
echo -e "Correctness validation: "
mpirun -np 4 ./poolingOP 4 2 "true"
echo -e "1024x1024 matrix 4x4 kernel 4 processors: "
mpirun -np 4 ./poolingOP 1024 4 "false"


echo -e "# WordCount "
mpicxx -o WordCountSmall WordCountSmall.cpp
mpicxx -o WordCountBig WordCountBig.cpp
echo -e "Small file: "
mpirun -np 3 ./WordCountSmall "small_file"
echo -e "Top 10 words in the 100 small files:"
sort -n -t ' ' -k 2 -r WordCountSmall_results.txt | head -n 10
echo -e "Big file: "
mpirun -np 3 ./WordCountBig "big_file/big_100.txt"
echo -e "Top 10 words in the big file:"
sort -n -t ' ' -k 2 -r WordCountBig_results.txt | head -n 10