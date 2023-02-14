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
const size_t SHIFTS[5] = {1,3,9,27,81};

static inline uint64_t rho(uint64_t x, int n) {
    uint64_t ret = 0;
    for (int i = 0; i < n; i++) {
        ret = ret * 3 + ((x >> (n - 1 - i)) & 1);
    }
    return ret;
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

static inline int log3(uint64_t n) {
    if (!n) {
        return -1;
    }
    int ret = 0;
    while (n > 1) {
        ret += 1;
        n /= 3;
    }
    return ret;
}

struct QuineMcCluskey {
    int n, h, n_h;
    size_t n_blocks;
    vector<BITSET3> S;
    
    QuineMcCluskey(int _n) {
        n = _n;
        h = BITSET3_LOG3;
        n_h = n - h;
        n_blocks = pow3(n-h);
        S.resize(n_blocks);
    }

    uint64_t get_RAM_usage() const {
        return S.size() * sizeof(BITSET3);
    }

    void reserve(size_t amount) {
        return;
    }
    
    void set(uint64_t xbin) {
        uint64_t xter = rho(xbin, n);
        S[xter / BITSET3_PER3][xter % BITSET3_PER3] = 1;
    }

    void run() {
        // MergeAll
        // --------
        
        // Top layer
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
                        S[i_u] |= S[i_s] & S[i_t];
                    }
                }
            }
        }

        // bottom layer
        {
            for (auto &Sa: S) {
                for(int i = 0; i < h; i++) {
                    auto mu = MASKS_TERNARY[i];
                    auto shift = SHIFTS[i];
                    
                    auto s = Sa & mu;
                    auto t = (Sa >> shift) & mu;
                    auto u = (Sa >> (2*shift)) & mu;
                    u |= s & t;
                    Sa = s | (t << shift) | (u << (2*shift));
                }
            }        
        }

        // ReduceAlls
        // --------
        
        // Top layer
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

        // bottom layer
        {
            for (auto &Sa: S) {
                for(int i = 0; i < h; i++) {
                    auto mu = MASKS_TERNARY[i];
                    auto shift = SHIFTS[i];

                    auto s = Sa & mu;
                    auto t = (Sa >> shift) & mu;
                    auto u = (Sa >> (2*shift)) & mu;
                    auto tmp = ~u;
                    s &= tmp;
                    t &= tmp;
                    Sa = s | (t << shift) | (u << (2*shift));
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
};


#include MAIN