CXX = g++

all: denseqmc sparseqmc sparseqmcext benchmark_sparseqmc benchmark_denseqmc benchmark_denseqmc_pure

benchmark_denseqmc_pure: dense_pure.cpp main_benchmark.cpp
	$(CXX) -O3 -march=native -std=c++2a dense_pure.cpp -o benchmark_denseqmc_pure -DMAIN='"main_benchmark.cpp"'

benchmark_denseqmc: dense.cpp main_benchmark.cpp
	$(CXX) -O3 -march=native -std=c++2a dense.cpp -o benchmark_denseqmc -DMAIN='"main_benchmark.cpp"'

benchmark_sparseqmc: sparse.cpp main_benchmark.cpp
	$(CXX) -O3 -march=native -std=c++2a sparse.cpp -o benchmark_sparseqmc -DMAIN='"main_benchmark.cpp"'

denseqmc: dense.cpp main_qmc.cpp
	$(CXX) -O3 -march=native -std=c++2a dense.cpp -o denseqmc -DMAIN='"main_qmc.cpp"'

sparseqmc: sparse.cpp main_qmc.cpp
	$(CXX) -O3 -march=native -std=c++2a sparse.cpp -o sparseqmc -DMAIN='"main_qmc.cpp"'

sparseqmcext: sparse.cpp main_qmc.cpp
	$(CXX) -O3 -march=native -std=c++2a -DN_EXT sparse.cpp -o sparseqmcext -DMAIN='"main_qmc.cpp"'