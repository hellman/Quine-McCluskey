#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage: %s <filename.in> [filename.out]\n", argv[0]);
		// fprintf(stderr, "Usage: %s {--cnf|--dnf} <filename.in> <filename.out>\n", argv[0]);
		// fprintf(stderr, "Input file should contain the truth table of the Boolean function using '0' and '1' ASCII characters.\n");
		fprintf(stderr, "Input file format:\n");
		// fprintf(stderr, "- The first line should contain the format letter of input a:\n");
		fprintf(stderr, "- The first line should contain the format letter and the dimension <n>:\n");
		fprintf(stderr, "  d - usual Quine-McCluskey (DNF mode)\n");
		fprintf(stderr, "  c - complement the input set (CNF mode)\n");
		fprintf(stderr, "- Each consequent line should consist of a bitstring of length <n>, containing only ASCII '0' or '1' characters.\n");
		return 0;
	}

	FILE *ifd = fopen(argv[1], "r");
	if (!ifd) {
		perror("Can not open input file:");
		return -1;
	}

	FILE *ofd;
	if (argc == 2 || !strcmp(argv[2], "-")) {
		ofd = stdout;
	}
	else {
		ofd = fopen(argv[2], "w");
		if (!ofd) {
			perror("Can not open output file:");
			return -1;
		}
	}

	// fseek(ifd, 0, SEEK_END);
	// size_t inp_size = ftell(ifd);
	// fseek(ifd, 0, SEEK_SET);

	// int n = 0;
	// while (inp_size > 1) {
	// 	n++;
	// 	inp_size >>= 1;
	// }
	// printf("n = %d reading %llu / %lu bytes from file\n", n, 1ull << n, inp_size);

	char mode;
	int n;
	if (fscanf(ifd, "%c %d\n", &mode, &n) != 2) {
		fprintf(stderr, "malformed first line\n");
		return -1;
	}
	fprintf(stderr, "n = %d mode = %c (%s)\n", n, mode, (mode == 'd') ? "DNF" : "CNF");
    fprintf(stderr, "bitslice alg needs RAM: %.2lf GiB\n", (double)pow3(n)*1.0 / 8 / 1e9);
    if (mode != 'c' && mode != 'd') {
    	fprintf(stderr, "unknown mode '%c' (need to be 'c' or 'd')\n", mode);
    	return -1;
    }
    if (n < 1 || n > N_MAX) {
    	fprintf(stderr, "n out of range [1, %d]\n", N_MAX);
    	return -1;
    }

	QuineMcCluskey D(n);
	size_t cnt = 0;
	uint64_t h = -1ull;	
    if (mode == 'd') {
    	printf("DNF mode uses sparse memory\n");
    	char linebuf[256] = {};
		while (1) {
			if (linebuf != fgets(linebuf, n+2, ifd)) {
				break;
			}
			uint64_t x = 0;
			for(int i = 0; i < n; i++) {
				if (linebuf[i] != '0' && linebuf[i] != '1') {
					fprintf(stderr, "read characted '%c' from file (line \"%s\"), expected '0' or '1'\n", linebuf[i], linebuf);
					return -1;
				}
				if (linebuf[i] == '1') {
					x |= 1ull << (n - 1 - i);
				}
			}
			// fprintf(stderr, "input %s -> %016lx\n", linebuf, x);
			if (x >= (1ull << n)) {
				fprintf(stderr, "too large value in the file?\n");
				return -1;
			}
			h += x;
			h *= 0xabc12def;
			h ^= (~h) >> 19; h ^= h << 17;
			cnt++;
			D.set(x);
		}
    }
    else {
    	// CNF mode, have to make the whole truthtable
    	printf("CNF mode requires full truth table construction (2^n memory)\n");
		vector<bool> function(1ull << n, (bool)(mode == 'c'));
		D.reserve(1ull << (n-1));
		char linebuf[256] = {};
		while (1) {
			if (linebuf != fgets(linebuf, n+2, ifd)) {
				break;
			}
			uint64_t x = 0;
			for(int i = 0; i < n; i++) {
				if (linebuf[i] != '0' && linebuf[i] != '1') {
					fprintf(stderr, "read characted '%c' from file (line \"%s\"), expected '0' or '1'\n", linebuf[i], linebuf);
					return -1;
				}
				if (linebuf[i] == '1') {
					x |= 1ull << (n - 1 - i);
				}
			}
			// fprintf(stderr, "input %s -> %016lx\n", linebuf, x);
			if (x >= function.size()) {
				fprintf(stderr, "too large value in the file?\n");
				return -1;
			}
			function[x] = (mode == 'd') ? 1 : 0;
		}
		
		for (uint64_t x = 0; x < (1ull << n); x++) {
			// char c = fgetc(ifd);
			h ^= (~h) >> 19; h ^= h << 17;
			// if (c == '1') {
			if (function[x]) {
				h += x;
				cnt++;
				D.set(x);
			}
			// else if (c != '0') {
			// 	fprintf(stderr, "unexpected symbol '%c' at offset %lu\n", c, x);
			// 	return -1;
			// }
		}
	}

	fprintf(stderr, "input size: %lu/%llu (density: %.1f%%)\n", cnt, 1ull<<n, cnt * 1.0 / (1ull << n));
	fprintf(stderr, "input checksum: %016lx\n", h);

	fprintf(stderr, "start n = %d\n", n);
	D.run();
	fprintf(stderr, "finished dense part\n");

	// char buf[100] = {};
	// assert(n < 99);	
	// buf[n] = '\n';
	cnt = 0;
	h = -1ull;
	fprintf(ofd, "p %s %d 0\n", (mode == 'c') ? "cnf" : "dnf", n);
	D.iter_sorted([&](HS_WORD xter) {
		cnt++;
		h ^= (~h) >> 19;
		h ^= h << 17;
		h += xter;

		auto [a, u] = unrho(xter, n);
		if (mode == 'c') {
			// CNF
			for (int i = 0; i < n; i++) {
				if ((u >> (n-1-i)) & 1) {
				}
				else if ((a >> (n-1-i)) & 1) {
					fprintf(ofd, "%d ", -(i+1));
				}
				else {
					fprintf(ofd, "%d ", (i+1));
				}
			}
		}
		else {
			// DNF
			for (int i = 0; i < n; i++) {
				if ((u >> (n-1-i)) & 1) {
				}
				else if ((a >> (n-1-i)) & 1) {
					fprintf(ofd, "%d ", (i+1));
				}
				else {
					fprintf(ofd, "%d ", -(i+1));
				}
			}
		}
		fprintf(ofd, "0\n");
		// 	if ((u >> (n-1-i)) & 1) {
		// 		buf[i] = '*';
		// 	}
		// 	else if ((a >> (n-1-i)) & 1) {
		// 		buf[i] = '1';
		// 	}
		// 	else {
		// 		buf[i] = '0';
		// 	}
		// }
		// fputs(buf, ofd);

	});
	fprintf(stderr, "output checksum: %016lx\n", h);
	fprintf(stderr, "out size: %lu\n", cnt);
    uint64_t RAM_bytes = D.get_RAM_usage();
    fprintf(stderr, "used RAM: %.2f GiB\n", RAM_bytes / (double)1e9);
	return 0;
}
