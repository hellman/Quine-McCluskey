#include <chrono>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s <n> <density>\n", argv[0]);
		printf("  1 <= n <= %d\n", N_MAX);
		printf("  0 <= density <= 100\n");
		return -1;
	}
	int n = atoi(argv[1]);
    int dens = atoi(argv[2]);

	printf("n = %d density = %d%%\n", n, dens);
    printf("bitslice alg needs RAM: %.2lf GiB\n", (double)pow3(n)*1.0 / 8 / 1e9);
    assert(1 <= n && n <= N_MAX);
    assert(0 <= dens <= 100);


	QuineMcCluskey D(n);
    D.reserve(1ull << (n-1));

	chrono::steady_clock::time_point begin = chrono::steady_clock::now();
	size_t cnt = 0;
	uint64_t h = -1ull;
	for (uint64_t x = 0; x < (1ull << n); x++) {
		h ^= (~h) >> 19; h ^= h << 17;
		if (rand() % 100 < dens) { 
			D.set(x);
			h += x;
			cnt++;
		}
	}
	printf("checksum: %016lx\n", h);
	printf("inp size: %lu\n", cnt);

	printf("start n = %d\n", n);
	fflush(stdout);
	D.run();
	
	cnt = 0;
	h = -1ull;
	D.iter([&](HS_WORD xter) {
		cnt++;
		h ^= (~h) >> 19;
		h ^= h << 17;
		h += xter;
	});
	chrono::steady_clock::time_point end = chrono::steady_clock::now();

	printf("checksum: %016lx\n", h);
	printf("out size: %lu\n", cnt);
    uint64_t RAM_bytes = D.get_RAM_usage();
    printf("elapsed RAM: %.2f GiB = %lu bytes\n", RAM_bytes / (double)(1ull << 30), RAM_bytes);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
    printf("elapsed TIME: %.3f hours = %lu nanoseconds\n", (uint64_t)seconds/3600.0, (uint64_t(ns)));
	return 0;
}
