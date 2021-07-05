echo -e "############### Project 2 ###############"
echo -e "# Monte Carlo\n"
g++ -fopenmp MonteCarlo.cpp -o MonteCarlo
./MonteCarlo 4 1000000

echo -e ""
echo -e "# Quick Sort\n"
g++ -fopenmp QuickSort.cpp -o QuickSort
echo -e "Correctness validation: "
./QuickSort 2 16 true
echo -e "\nSort 1000000 elements, 2 threads: "
./QuickSort 2 1000000 false

echo -e ""
echo -e "# PageRank\n"
g++ -fopenmp PageRank.cpp -o PageRank
echo -e "Correctness validation: "
./PageRank 4 20 3 1 true
echo -e "\n1024000 nodes 100 iterations 4 threads: "
./PageRank 4 1024000 10 100 false
