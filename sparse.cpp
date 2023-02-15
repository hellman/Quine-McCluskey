#include <bitset>
#include <vector>
#include <functional>
#include <unordered_set>
#include <assert.h>

#ifndef N_EXT
constexpr __uint128_t ONE = 1;
constexpr __uint128_t TWO = 2;
constexpr __uint128_t THREE = 3;
constexpr __uint128_t HS_DEL_MARK = (3*ONE) << 126;
constexpr __uint128_t HS_VALUE = ONE << 126;
constexpr __uint128_t HS_VALUE_MASK = (ONE<<126)-1;
typedef __uint128_t HS_WORD;
constexpr int N_MAX = 31;
#else
constexpr uint64_t ONE = 1;
constexpr uint64_t TWO = 2;
constexpr uint64_t THREE = 3;
constexpr uint64_t HS_DEL_MARK = 3ull << 62;
constexpr uint64_t HS_VALUE = 1ull << 62;
constexpr uint64_t HS_VALUE_MASK = (1ull << 62)-1;
typedef uint64_t HS_WORD;
constexpr int N_MAX = 63;
#endif

using namespace std;

static inline HS_WORD rho(uint64_t x, int n) {
    HS_WORD ret = 0;
    for (int i = 0; i < n; i++) {
        ret = (ret << 2) | ((x >> (n - 1 - i)) & 1);
    }
    return ret;
}
static inline pair<uint64_t, uint64_t> unrho(HS_WORD x, int n) {
    uint64_t a = 0, u = 0;
    for (int i = 0; i < n; i++) {
        uint64_t digit = x % 3;
        x /= 3;
        if (digit == 1) {
            a ^= 1ull << i;
        }
        else if (digit == 2) {
            u ^= 1ull << i;
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

// Optimized Hash-based unordered set
// warning: this is only a 2-stage HashSet
// (only insertions, then only marks/removals)
struct HashSet {
    int n;
    uint64_t mask;
    // two MSB:
    // 00 - empty
    // 01 - value
    // 11 - marked (deleted)
    vector<HS_WORD> space;
    vector<HS_WORD> contents;

    HashSet() {
        int bits = 4; // auto-growing
        n = bits;
        space.resize(1ull << bits);
        mask = space.size()-1;
    }

    static uint64_t hash(HS_WORD x) {
        x ^= 0x53d4cdfafda2f0b1ull;
        #ifdef N_EXT
            x *= 0xcdfafda2f0b13ef7ull;
            x ^= x >> 60;
        #endif
        x *= 0x7fabcdef;
        x ^= x >> 11;
        x ^= x >> 27;
        x *= 0x7f17316b;
        x ^= x >> 11;
        x ^= x >> 27;
        x ^= x >> 17;
        x ^= x >> 13;
        return (uint64_t)x;
    }
    void reserve(size_t amount) {
        amount *= 2;
        int e = 0;
        while (amount) {
            amount >>= 1;
            e++;
        }
        if (e > n) {
            widen(e-n);
        }
    }
    void widen(int add_bits=1) {
        printf("widening hash space from %d to %d bits\n", n, n + add_bits);
        n += add_bits;
        space.assign(1ull << n, 0);
        mask = space.size()-1;
        for(auto x: contents) {
            insert(x, false);
        }
    }

    void insert(HS_WORD x, bool add_to_contents=true) {
        // WARNING: no insertions after deletions!!!
        uint64_t h = hash(x);
        uint64_t idx = h & mask;
        HS_WORD val = x | HS_VALUE;
        // printf("insert %016lx %016lx\n", x, idx);
        int itr = 0;
        while (space[idx] != 0) {
            if (space[idx] == val) {
                return;
            }
            idx = (idx + 1) & mask;

            if (++itr == 50) {
                widen();
                idx = h & mask;
            }
        }
        space[idx] = val;
        if (add_to_contents) {
            contents.push_back(x);
        }
        if (contents.size() >= space.size()) {
            printf("hash-set full, force widen\n");
            widen();
        }
        // printf("insert ok\n");
    }

    HS_WORD find(HS_WORD x) {
        uint64_t idx = hash(x) & mask;
        // printf("find %016lx %016lx\n", x, idx);
        while (space[idx] != 0) {
            if ((space[idx] & HS_VALUE_MASK) == x) {
                return space[idx];
            }
            idx = (idx + 1) & mask;
        }
        return 0;
    }

    void mark(HS_WORD x) {
        uint64_t idx = hash(x) & mask;
        HS_WORD val = x | HS_VALUE;
        HS_WORD marked_val = x | HS_DEL_MARK;
        // printf("mark %016lx %016lx\n", x, idx);
        while (space[idx] != 0) {
            if (space[idx] == val || space[idx] == marked_val) {
                space[idx] = marked_val;
                return;
            }
            idx = (idx + 1) & mask;
        }
    }
    vector<HS_WORD> list() {
        vector<HS_WORD> ret;
        ret.reserve(contents.size());
        // if (0) {
        //     for (auto x: space) {
        //         if (x == x | HS_VALUE && x != x | HS_DEL_MARK) {
        //             ret.push_back(x);
        //         }
        //     }
        // }
        // else {
        for (auto x: contents) {
            if (find(x) == (x | HS_VALUE)) {
                ret.push_back(x);
            }
        }
        // }
        return ret;
    }

    void clear() {
        space.assign(1ull << n, 0);
        contents.clear();
    }
};
void swap(HashSet& a, HashSet& b) {
    swap(a.space, b.space);
    swap(a.contents, b.contents);
    swap(a.mask, b.mask);
    swap(a.n, b.n);
}

struct QuineMcCluskey {
    int n;
    HashSet S;
    HashSet S2;
    vector<uint64_t> primes;
    
    QuineMcCluskey(int _n) {
        n = _n;
        assert(n <= N_MAX);
    }

    void reserve(size_t amount) {
        S.reserve(amount);
        // S2.reserve(amount);
        primes.reserve(amount);
    }

    uint64_t get_RAM_usage() const {
        return S.space.size() * 8 + S2.space.size() * 8;
    }

    void set(uint64_t xbin) {
        HS_WORD xter = rho(xbin, n);
        S.insert(xter);
    }

    void run() {
        for (int w = 0; w < n; w++) {
            // printf("weight %d todo %lu\n", w, S.contents.size());
            vector<HS_WORD> news;
            news.reserve(S.contents.size());
            for (auto s: S.contents) { // nothing marked yet
                auto ss = s;
                int stopped = 0;
                for (int i = 0; i < n; i++) { 
                    auto type = ss & 3;
                    if (type == 0) { // 00 -> 0
                        HS_WORD t = s ^ (ONE << (2 * i)); // 01 -> 1
                        if (S.find(t)) {
                            HS_WORD u = s ^ (TWO << (2 * i)); // 10 -> *
                            // S2.insert(u);
                            // S.mark(s);
                            // S.mark(t);
                            news.push_back(u);
                        }
                    }
                    else if (type == 2) { // 10 -> *
                        // only the leftmost star should be considered
                        // (others are duplicates)
                        // (though this requires another loop over S2 to mark redundant ones in S)
                        break;
                    }
                    ss >>= 2;
                }
            }

            printf("weight %d new minterms %lu\n", w, news.size());
            // collected implicants list
            // allows to reserve the required set size in one step
            // and to insert each implicant exactly once (instead of w times)
            S2.reserve(news.size());
            for (auto u: news) { // nothing marked yet
                S2.insert(u);
                for (int i = 0; i < n; i++) { 
                    auto type = (u >> (2*i))&3;
                    if (type == 2) { // 10 -> *
                        auto s = u ^ (TWO << (2*i)); // 00 -> 0
                        auto t = u ^ (THREE << (2*i)); // 01 -> 1
                        S.mark(s);
                        S.mark(t);
                    }
                }
            }

            for (auto s: S.list()) { // skip marked
                primes.push_back(s);
            }
            
            S.clear();
            swap(S, S2);
            
            if (S.contents.size() == 0) {
                break;
            }
        }
        for (auto u: S.contents) {
            primes.push_back(u);
        }
        S.clear();
    }
    void iter_sorted(function<void(HS_WORD x)> const & func) const {
        // compatible version of iteration order (sorted by interna repr.)
        
        // iterate over points (compressed ternary)
        vector<HS_WORD> srt;
        for (auto x: primes) {
            HS_WORD xter = 0;
            for (int i = 0; i < n; i++) {
                int shift = 2*(n-1-i);
                auto val = ((x >> shift)& 3);
                assert(0 <= val <= 2);
                xter = (xter * 3) + ((x >> shift)& 3);
            }
            srt.push_back(xter);
        }

        sort(srt.begin(), srt.end());
        for (auto x: srt) {
            func(x);
        }
    }
    void iter(function<void(HS_WORD x)> const & func) const {
        // noncompatible version of iteration order (sorted by weights but ties are not stable)
        
        // iterate over points (compressed ternary)
        for (auto x: primes) {
            HS_WORD xter = 0;
            for (int i = 0; i < n; i++) {
                int shift = 2*(n-1-i);
                auto val = ((x >> shift)& 3);
                assert(0 <= val <= 2);
                xter = (xter * 3) + ((x >> shift)& 3);
            }
            func(xter);
        }
    }
};


#include MAIN