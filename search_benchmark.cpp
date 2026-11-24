// COMP 3305 Project: Optimized search techniques
// Binary search, Interpolation search and Exponential search on 100,000+ records.
//
// Build:  g++ -O2 -std=c++17 -o search_benchmark search_benchmark.cpp
// Run:    ./search_benchmark            (writes records_*.csv and results.csv)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// ------------------------------------------------------------------ data
struct Record {
    long long id;     // search key (unique, sorted ascending)
    string name;
    double gpa;
};

// ------------------------------------------------------- the 3 algorithms
// Each returns the index of `key` or -1. `probes` counts key comparisons
// against array elements (only when Count == true, so timing runs stay clean).

template <bool Count>
int binarySearch(const vector<Record>& a, long long key, int lo, int hi, uint64_t& probes) {
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (Count) ++probes;
        if (a[mid].id == key) return mid;
        if (a[mid].id < key) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

template <bool Count>
int interpolationSearch(const vector<Record>& a, long long key, uint64_t& probes) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi && key >= a[lo].id && key <= a[hi].id) {
        if (a[hi].id == a[lo].id) {                      // avoid divide-by-zero
            if (Count) ++probes;
            return a[lo].id == key ? lo : -1;
        }
        double frac = (double)(key - a[lo].id) / (double)(a[hi].id - a[lo].id);
        int pos = lo + (int)(frac * (hi - lo));          // the "interpolated" guess
        if (Count) ++probes;
        if (a[pos].id == key) return pos;
        if (a[pos].id < key) lo = pos + 1; else hi = pos - 1;
    }
    return -1;
}

template <bool Count>
int exponentialSearch(const vector<Record>& a, long long key, uint64_t& probes) {
    int n = (int)a.size();
    if (n == 0) return -1;
    if (Count) ++probes;
    if (a[0].id == key) return 0;
    int bound = 1;                                       // gallop: 1,2,4,8,...
    while (bound < n && a[bound].id < key) {
        if (Count) ++probes;
        bound *= 2;
    }
    if (Count && bound < n) ++probes;                    // the failing check
    return binarySearch<Count>(a, key, bound / 2, min(bound, n - 1), probes);
}

// ------------------------------------------------------- dataset builders
// Records are generated, saved to CSV, then re-loaded (so the program really
// works from a file of >= 100,000 records).
vector<Record> makeUniform(int n, mt19937_64& rng) {
    uniform_int_distribution<int> gap(1, 9);             // nearly evenly spaced IDs
    uniform_real_distribution<double> g(2.0, 4.0);
    vector<Record> v; v.reserve(n);
    long long id = 1000000;
    for (int i = 0; i < n; ++i) {
        id += gap(rng);
        v.push_back({id, "Student_" + to_string(i), round(g(rng) * 100) / 100});
    }
    return v;
}
vector<Record> makeSkewed(int n, mt19937_64& rng) {      // keys grow exponentially
    uniform_real_distribution<double> g(2.0, 4.0);
    vector<Record> v; v.reserve(n);
    for (int i = 0; i < n; ++i) {
        long long id = (long long)i + (long long)pow(1.0001, i);
        v.push_back({id, "Item_" + to_string(i), round(g(rng) * 100) / 100});
    }
    return v;
}
void saveCSV(const string& f, const vector<Record>& v) {
    ofstream o(f); o << "id,name,gpa\n";
    for (auto& r : v) o << r.id << ',' << r.name << ',' << r.gpa << '\n';
}
vector<Record> loadCSV(const string& f) {
    ifstream in(f); string line; getline(in, line);
    vector<Record> v;
    while (getline(in, line)) {
        stringstream ss(line); string a, b, c;
        getline(ss, a, ','); getline(ss, b, ','); getline(ss, c, ',');
        v.push_back({stoll(a), b, stod(c)});
    }
    sort(v.begin(), v.end(), [](auto& x, auto& y) { return x.id < y.id; });
    return v;
}

// ------------------------------------------------------------- benchmark
struct Result { double probes; double ns; };

template <class F>
Result measure(const vector<long long>& queries, F f) {
    uint64_t probes = 0, sink = 0;
    for (long long q : queries) sink += f(q, probes, true);      // counting pass
    // timing pass (no counter overhead); repeat 3x, keep the fastest
    double best = 1e30;
    for (int rep = 0; rep < 3; ++rep) {
        uint64_t dummy = 0;
        auto t0 = chrono::steady_clock::now();
        for (long long q : queries) sink += f(q, dummy, false);
        auto t1 = chrono::steady_clock::now();
        best = min(best, chrono::duration<double, nano>(t1 - t0).count() / queries.size());
    }
    if (sink == 42) cerr << "";                                  // keep optimiser honest
    return {(double)probes / queries.size(), best};
}

struct Trio { Result bin, interp, expo; };

Trio runAll(const vector<Record>& a, const vector<long long>& q) {
    Trio t;
    t.bin = measure(q, [&](long long k, uint64_t& p, bool c) {
        return c ? binarySearch<true>(a, k, 0, (int)a.size() - 1, p)
                 : binarySearch<false>(a, k, 0, (int)a.size() - 1, p); });
    t.interp = measure(q, [&](long long k, uint64_t& p, bool c) {
        return c ? interpolationSearch<true>(a, k, p) : interpolationSearch<false>(a, k, p); });
    t.expo = measure(q, [&](long long k, uint64_t& p, bool c) {
        return c ? exponentialSearch<true>(a, k, p) : exponentialSearch<false>(a, k, p); });
    return t;
}

// correctness: all three algorithms must agree with std::lower_bound
bool verify(const vector<Record>& a, mt19937_64& rng) {
    uint64_t p = 0;
    uniform_int_distribution<int> pick(0, (int)a.size() - 1);
    for (int i = 0; i < 20000; ++i) {
        long long k = a[pick(rng)].id;
        if (i % 4 == 0) k += 1;                                   // some may be misses
        auto it = lower_bound(a.begin(), a.end(), k, [](const Record& r, long long v) { return r.id < v; });
        int expect = (it != a.end() && it->id == k) ? int(it - a.begin()) : -1;
        if (binarySearch<false>(a, k, 0, (int)a.size() - 1, p) != expect) return false;
        if (interpolationSearch<false>(a, k, p) != expect) return false;
        if (exponentialSearch<false>(a, k, p) != expect) return false;
    }
    return true;
}

int main() {
    mt19937_64 rng(3305);
    ofstream res("results.csv");
    res << "experiment,n,algorithm,avg_probes,ns_per_query\n";
    auto row = [&](const string& exp, int n, const Trio& t) {
        auto w = [&](const char* name, const Result& r) {
            res << exp << ',' << n << ',' << name << ',' << r.probes << ',' << r.ns << '\n';
            cout << left << setw(26) << exp << setw(10) << n << setw(15) << name
                 << right << setw(9) << fixed << setprecision(2) << r.probes << " probes"
                 << setw(10) << setprecision(1) << r.ns << " ns/query\n";
        };
        w("Binary", t.bin); w("Interpolation", t.interp); w("Exponential", t.expo);
    };

    const int N = 200000;                                         // main dataset
    saveCSV("records_uniform.csv", makeUniform(N, rng));
    saveCSV("records_skewed.csv",  makeSkewed(N, rng));
    auto uni = loadCSV("records_uniform.csv");
    auto skw = loadCSV("records_skewed.csv");
    cout << "Loaded " << uni.size() << " uniform and " << skw.size() << " skewed records\n";
    cout << "Verification: " << (verify(uni, rng) && verify(skw, rng) ? "PASSED" : "FAILED") << "\n\n";

    int Q = 1000000;                                              // queries per experiment
    auto randomKeys = [&](const vector<Record>& a, int lo, int hi) {
        uniform_int_distribution<int> d(lo, hi);
        vector<long long> q(Q);
        for (auto& x : q) x = a[d(rng)].id;
        return q;
    };
    // 1. random existing keys, uniformly spread data
    row("uniform_random", N, runAll(uni, randomKeys(uni, 0, N - 1)));
    // 2. keys among the first 100 records (target near the front)
    row("uniform_front100", N, runAll(uni, randomKeys(uni, 0, 99)));
    // 3. keys that are not in the data (unsuccessful search)
    { vector<long long> q; uniform_int_distribution<int> d(0, N - 2);
      while ((int)q.size() < Q) {                                 // pick a key strictly inside a gap
          int i = d(rng);
          if (uni[i + 1].id - uni[i].id >= 2) q.push_back(uni[i].id + 1);
      }
      row("uniform_miss", N, runAll(uni, q)); }
    // 4. skewed (exponentially growing) keys, random targets.
    //    Interpolation search is close to linear here, so use fewer queries.
    Q = 20000;
    row("skewed_random", N, runAll(skw, randomKeys(skw, 0, N - 1)));
    // 5. scaling: dataset size (uniform data, random keys)
    Q = 1000000;
    for (int n : {100000, 200000, 500000, 1000000, 2000000}) {
        auto v = makeUniform(n, rng);
        row("scale", n, runAll(v, randomKeys(v, 0, n - 1)));
    }
    return 0;
}
