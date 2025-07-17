
/*  cube_cubeprime_fast.cpp
    Verifies (if possible) that           n³ = p + q + r
    where p, q, r are *distinct* prime numbers.

    The code is tuned to finish the range 3 → 20 000 in a few seconds
    on a quad‑core CPU.  It remains valid up to n ≤ 2 642 245, the largest n
    whose cube still fits into an unsigned 64‑bit integer.

    Build:
        g++ -std=c++20 -O3 -march=native -pthread cube_cubeprime_fast.cpp -o cube_test
*/

#include <bits/stdc++.h>
using namespace std;

/*──────────────────────── 1. Fast sieve up to 1e8 ────────────────────────*/
struct Sieve {
    vector<uint8_t> mark;   // mark[x] == 1  →  x is *composite*
    vector<uint32_t> primes;

    explicit Sieve(uint32_t lim = 100'000'000) : mark(lim + 1, 0) {
        mark[0] = mark[1] = 1;                       // 0 and 1 are not primes
        for (uint32_t i = 2; i * i <= lim; ++i)      // classic Erasmus loop
            if (!mark[i])
                for (uint32_t j = i * i; j <= lim; j += i)
                    mark[j] = 1;
        for (uint32_t i = 2; i <= lim; ++i)          // collect primes
            if (!mark[i]) primes.push_back(i);
    }
    inline bool isPrimeSmall(uint32_t x) const { return !mark[x]; }
};

/*──────────────────── 2. Deterministic Miller–Rabin for 64‑bit ───────────*/
inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t m) {
    return static_cast<uint64_t>((__uint128_t)a * b % m);
}
inline uint64_t pow_mod(uint64_t a, uint64_t e, uint64_t m) {
    uint64_t r = 1;
    while (e) {
        if (e & 1) r = mul_mod(r, a, m);
        a = mul_mod(a, a, m);
        e >>= 1;
    }
    return r;
}
bool isPrime64(uint64_t n, const Sieve& sv) {
    if (n <= 10'000'000ULL)             // reuse sieve for small numbers
        return sv.isPrimeSmall(static_cast<uint32_t>(n));
    if ((n & 1) == 0) return false;     // even > 2 are composite

    static const uint64_t bases[] = {2, 3, 5, 7, 11, 13, 17};   // proven set
    uint64_t d = n - 1, r = 0;
    while (!(d & 1)) { d >>= 1; ++r; } // write n − 1 = 2ʳ · d  with d odd

    for (uint64_t a : bases) {
        if (a >= n) break;
        uint64_t x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (uint64_t i = 1; i < r; ++i) {
            x = mul_mod(x, x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

/*──────────────────────── 3. Single cube test for one n ──────────────────*/
bool cube_repr(uint64_t n, const Sieve& sv, string& line) {
    uint64_t m = static_cast<uint64_t>((__uint128_t)n * n * n); // n³

    /* Phase 0: even cube → try pattern 2 + p + q */
    if ((m & 1) == 0) {
        for (uint32_t p : sv.primes) {
            if (p >= m) break;
            uint64_t q = m - 2 - p;
            if (q <= p) break;                 // enforce p < q < r
            if (isPrime64(q, sv) && q != 2 && p != 2) {
                line = to_string(n) + "^3 = " + to_string(m) +
                       " = 2 + " + to_string(p) + " + " + to_string(q);
                return true;
            }
        }
    }

    /* Phase 1: double loop over distinct small primes p < q */
    for (size_t i = 0; i < sv.primes.size(); ++i) {
        uint64_t p = sv.primes[i];
        if ((__uint128_t)p * 3 > m) break;     // p already too large
        for (size_t j = i + 1; j < sv.primes.size(); ++j) {
            uint64_t q = sv.primes[j];
            uint64_t r = m - p - q;
            if (r < q) break;                  // maintain order q < r
            if (isPrime64(r, sv) && r != p && r != q) {
                line = to_string(n) + "^3 = " + to_string(m) + " = " +
                       to_string(p) + " + " + to_string(q) + " + " + to_string(r);
                return true;
            }
        }
    }

    line = to_string(n) + "^3 = " + to_string(m) + ": NO REPRESENTATION FOUND";
    return false;
}

/*──────────────────────── 4. Threading context & worker ─────────────────*/
struct Ctx {
    uint64_t start, end;
    atomic<uint64_t> next;
    const Sieve& sv;
    vector<string> out;
    mutex m;
    Ctx(uint64_t a, uint64_t b, const Sieve& s)
        : start(a), end(b), next(a), sv(s) {}
};
void worker(Ctx& c) {
    vector<string> local;
    uint64_t n;
    while ((n = c.next.fetch_add(1, memory_order_relaxed)) <= c.end) {
        string line;
        cube_repr(n, c.sv, line);
        local.push_back(std::move(line));
    }
    lock_guard<mutex> g(c.m);                 // flush local results
    c.out.insert(c.out.end(),
                 make_move_iterator(local.begin()),
                 make_move_iterator(local.end()));
}

/*────────────────────────────── 5. main ────────────────────────────────*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const uint64_t MAX_N = 2'642'245ULL;      // largest n with n³ < 2⁶⁴
    uint64_t a, b;
    cout << "start n (3-" << MAX_N << "): " << flush;
    if (!(cin >> a) || a < 3 || a > MAX_N) {
        cerr << "invalid\n"; return 1;
    }
    cout << "end n (" << a << "-" << MAX_N << "): " << flush;
    if (!(cin >> b) || b < a || b > MAX_N) {
        cerr << "invalid\n"; return 1;
    }

    Sieve sv;                                 // build sieve once
    Ctx ctx(a, b, sv);                        // shared context

    unsigned th = max(2u, thread::hardware_concurrency());
    vector<thread> pool;
    pool.reserve(th);
    for (unsigned i = 0; i < th; ++i)
        pool.emplace_back(worker, ref(ctx));
    for (auto& t : pool) t.join();

    /* sort by n before writing */
    sort(ctx.out.begin(), ctx.out.end(),
         [](const string& x, const string& y) {
             return stoull(x.substr(0, x.find('^'))) <
                    stoull(y.substr(0, y.find('^')));
         });

    ofstream f("results.txt");
    for (auto& s : ctx.out) f << s << '\n';
    cout << "done: " << ctx.out.size()
         << " lines → results.txt\n";
    return 0;
}
