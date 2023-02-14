#include <bitset>
#include <vector>
#include <functional>
#include <assert.h>

using namespace std;

typedef uint64_t HS_WORD;
constexpr int N_MAX = 27; // increase if you have more than 1TiB RAM ...

// 2^8 = 256 bits for 3^5 = 243 bits
static constexpr uint64_t BITSET3_LOG2 = 8;
static constexpr uint64_t BITSET3_LOG3 = 5;
static constexpr uint64_t BITSET3_PER3 = 243;
typedef std::bitset<1ull << BITSET3_LOG2> BITSET3;

const BITSET3 MASKS_TERNARY[5] = {
    // note: bitset(char *) is given MSB to LSB, snapped to LSB (missing MSBs are filled with zeroes)
    BITSET3("001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001001"),
    BITSET3("000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111000000111"),
    BITSET3("000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111000000000000000000111111111"),
    BITSET3("000000000000000000000000000000000000000000000000000000111111111111111111111111111000000000000000000000000000000000000000000000000000000111111111111111111111111111000000000000000000000000000000000000000000000000000000111111111111111111111111111"),
    BITSET3("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111"),
};
constexpr size_t SHIFTS[5] = {1,3,9,27,81};

// sage: tab = [sum(c*3**i for i, c in enumerate(ZZ(x).digits(2))) for x in range(2**8)]
constexpr uint64_t RHO_TAB[256] = {0, 1, 3, 4, 9, 10, 12, 13, 27, 28, 30, 31, 36, 37, 39, 40, 81, 82, 84, 85, 90, 91, 93, 94, 108, 109, 111, 112, 117, 118, 120, 121, 243, 244, 246, 247, 252, 253, 255, 256, 270, 271, 273, 274, 279, 280, 282, 283, 324, 325, 327, 328, 333, 334, 336, 337, 351, 352, 354, 355, 360, 361, 363, 364, 729, 730, 732, 733, 738, 739, 741, 742, 756, 757, 759, 760, 765, 766, 768, 769, 810, 811, 813, 814, 819, 820, 822, 823, 837, 838, 840, 841, 846, 847, 849, 850, 972, 973, 975, 976, 981, 982, 984, 985, 999, 1000, 1002, 1003, 1008, 1009, 1011, 1012, 1053, 1054, 1056, 1057, 1062, 1063, 1065, 1066, 1080, 1081, 1083, 1084, 1089, 1090, 1092, 1093, 2187, 2188, 2190, 2191, 2196, 2197, 2199, 2200, 2214, 2215, 2217, 2218, 2223, 2224, 2226, 2227, 2268, 2269, 2271, 2272, 2277, 2278, 2280, 2281, 2295, 2296, 2298, 2299, 2304, 2305, 2307, 2308, 2430, 2431, 2433, 2434, 2439, 2440, 2442, 2443, 2457, 2458, 2460, 2461, 2466, 2467, 2469, 2470, 2511, 2512, 2514, 2515, 2520, 2521, 2523, 2524, 2538, 2539, 2541, 2542, 2547, 2548, 2550, 2551, 2916, 2917, 2919, 2920, 2925, 2926, 2928, 2929, 2943, 2944, 2946, 2947, 2952, 2953, 2955, 2956, 2997, 2998, 3000, 3001, 3006, 3007, 3009, 3010, 3024, 3025, 3027, 3028, 3033, 3034, 3036, 3037, 3159, 3160, 3162, 3163, 3168, 3169, 3171, 3172, 3186, 3187, 3189, 3190, 3195, 3196, 3198, 3199, 3240, 3241, 3243, 3244, 3249, 3250, 3252, 3253, 3267, 3268, 3270, 3271, 3276, 3277, 3279, 3280};

static inline uint64_t rho(uint64_t x, int n) {
    if (0) {
        // optimized, not useful
        uint64_t ret = 0, mult = 1;
        while (x) {
            ret = ret + mult * RHO_TAB[x & 0xff];
            mult *= 6561;
            x >>= 8;
        }
        return ret;
    }
    else {
        uint64_t ret = 0;
        for (int i = 0; i < n; i++) {
            ret = ret * 3 + ((x >> (n - 1 - i)) & 1);
        }
        return ret;
    }
}
static inline pair<uint64_t, uint64_t> unrho(uint64_t x, int n) {
    uint64_t a = 0, u = 0;
    for (int i = 0; i < n; i++) {
        uint64_t digit = x % 3;
        x /= 3;
        if (digit == 1) {
            a ^= 1 << i;
        }
        else if (digit == 2) {
            u ^= 1 << i;
        }
    }
    return {a, u};
}

static inline uint64_t pow3(int e) {
    uint64_t ret = 1;
    uint64_t cur = 3;
    while (e) {
        if (e & 1) {
            ret = ret * cur;
        }
        e >>= 1;
        cur = cur * cur;
    }
    return ret;
}



struct QuineMcCluskey {
    int n, h, n_h;
    size_t n_blocks;
    vector<BITSET3> S;
    
    QuineMcCluskey(int _n) {
        h = BITSET3_LOG3;
        n = max(_n, h); // we will do full block processing anyway
        n_h = n - h;
        n_blocks = pow3(n-h);
        S.resize(n_blocks);
    }

    void reserve(size_t amount) {
        return;
    }

    uint64_t get_RAM_usage() const {
        return S.size() * sizeof(BITSET3);
    }

    void set(uint64_t xbin) {
        uint64_t xter = rho(xbin, n);
        S[xter / BITSET3_PER3][xter % BITSET3_PER3] = 1;
    }

    void run() {        
        // merge: top layer
        {
            size_t step = 1;
            for (int i = 1; i <= n_h; i++) {
                size_t shift = step;
                step *= 3;
                for (size_t b = 0; b < n_blocks; b += step) {
                    for (size_t c = 0; c < shift; c++) {
                        size_t i_s = b + c;
                        size_t i_t = i_s + shift;
                        size_t i_u = i_t + shift;
                        // printf("i%d: %lu %lu %lu ? %lu \n", i, b, c, i_u, S.size());
                        // assert(i_u < S.size());
                        S[i_u] |= S[i_s] & S[i_t];
                    }
                }
            }
        }

        // bottom layer: merge then reduce (can be interleaved since block-wise)
        {
            for (auto &Sa: S) {
                // all merges
                // unrolled & optimized loop
                BITSET3 s, t;

                s = Sa & MASKS_TERNARY[0];
                t = (Sa >> 1) & MASKS_TERNARY[0];
                Sa |= (s & t) << 2;

                s = Sa & MASKS_TERNARY[1];
                t = (Sa >> 3) & MASKS_TERNARY[1];
                Sa |= (s & t) << 6;

                s = Sa & MASKS_TERNARY[2];
                t = (Sa >> 9) & MASKS_TERNARY[2];
                Sa |= (s & t) << 18;

                s = Sa & MASKS_TERNARY[3];
                t = (Sa >> 27) & MASKS_TERNARY[3];
                Sa |= (s & t) << 54;

                s = Sa & MASKS_TERNARY[4];
                t = (Sa >> 81) & MASKS_TERNARY[4];
                Sa |= (s & t) << 162;
            
                // all reduce
                // unrolled & optimized loop
                BITSET3 u;

                u = ((Sa >> 2) & MASKS_TERNARY[0]);
                Sa &= ~(u | (u << 1));
                
                u = ((Sa >> 6) & MASKS_TERNARY[1]);
                Sa &= ~(u | (u << 3));
                
                u = ((Sa >> 18) & MASKS_TERNARY[2]);
                Sa &= ~(u | (u << 9));
                
                u = ((Sa >> 54) & MASKS_TERNARY[3]);
                Sa &= ~(u | (u << 27));
                
                u = ((Sa >> 162) & MASKS_TERNARY[4]);
                Sa &= ~(u | (u << 81));
            }        
        }
        
        // reduce: top layer
        {
            size_t step = 1;
            for (int i = 1; i <= n_h; i++) {
                size_t shift = step;
                step *= 3;
                for (size_t b = 0; b < n_blocks; b += step) {
                    for (size_t c = 0; c < shift; c++) {
                        size_t i_s = b + c;
                        size_t i_t = i_s + shift;
                        size_t i_u = i_t + shift;
                        auto tmp = ~S[i_u];
                        S[i_s] &= tmp;
                        S[i_t] &= tmp;
                    }
                }
            }
        }

    }

    void iter(function<void(uint64_t x)> const & func) const {
        // iterate over points (compressed ternary)
        vector<uint64_t> output;
        for(size_t hi = 0; hi < S.size(); hi++) {
            if (S[hi].any()) {
                auto &w = S[hi];
                size_t hi3 = hi * BITSET3_PER3;
                for(int lo = 0; lo < (int)BITSET3_PER3; lo++) {
                    if (w[lo]) {
                        func(hi3 + lo);
                    }
                }
            }
        }
    }

    void iter_sorted(function<void(uint64_t x)> const & func) const {
        // iterate over points (compressed ternary)
        vector<uint64_t> output;
        for(size_t hi = 0; hi < S.size(); hi++) {
            if (S[hi].any()) {
                auto &w = S[hi];
                size_t hi3 = hi * BITSET3_PER3;
                for(int lo = 0; lo < (int)BITSET3_PER3; lo++) {
                    if (w[lo]) {
                        func(hi3 + lo);
                    }
                }
            }
        }
    }
};

#include MAIN