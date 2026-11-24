// All three optimized search techniques in one program, on 150,000 sorted records.
// Build: g++ -O2 -std=c++17 search_all.cpp -o search_all
// Run:   ./search_all            (demo + benchmark)
//        ./search_all 123456     (search for a specific id with all three)
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

struct Record { long long id; string name; double gpa; };

vector<Record> makeDataset(int n) {                  // sorted by id (ids spaced 1..9 apart)
    mt19937 rng(3305);
    uniform_int_distribution<int> gap(1, 9);
    uniform_real_distribution<double> g(2.0, 4.0);
    vector<Record> v; v.reserve(n);
    long long id = 100000;
    for (int i = 0; i < n; ++i) {
        id += gap(rng);
        v.push_back({id, "Student_" + to_string(i), int(g(rng) * 100) / 100.0});
    }
    return v;
}

// ---------------------------------------------------------------- 1. Binary search
int binarySearch(const vector<Record>& a, long long key, int low, int high, int& steps) {
    while (low <= high) {
        int mid = low + (high - low) / 2;
        ++steps;
        if (a[mid].id == key)      return mid;
        else if (a[mid].id < key)  low = mid + 1;
        else                       high = mid - 1;
    }
    return -1;
}
int binarySearch(const vector<Record>& a, long long key, int& steps) {
    steps = 0;
    return binarySearch(a, key, 0, (int)a.size() - 1, steps);
}

// ---------------------------------------------------------------- 2. Interpolation search
int interpolationSearch(const vector<Record>& a, long long key, int& steps) {
    int low = 0, high = (int)a.size() - 1;
    steps = 0;
    while (low <= high && key >= a[low].id && key <= a[high].id) {
        if (a[low].id == a[high].id) { ++steps; return a[low].id == key ? low : -1; }
        double fraction = double(key - a[low].id) / double(a[high].id - a[low].id);
        int pos = low + int(fraction * (high - low));
        ++steps;
        if (a[pos].id == key)      return pos;
        else if (a[pos].id < key)  low = pos + 1;
        else                       high = pos - 1;
    }
    return -1;
}

// ---------------------------------------------------------------- 3. Exponential search
int exponentialSearch(const vector<Record>& a, long long key, int& steps) {
    int n = (int)a.size();
    steps = 0;
    if (n == 0) return -1;
    ++steps;
    if (a[0].id == key) return 0;
    int bound = 1;
    while (bound < n && a[bound].id < key) { ++steps; bound *= 2; }
    return binarySearch(a, key, bound / 2, min(bound, n - 1), steps);
}

// ---------------------------------------------------------------- demo helpers
void searchAll(const vector<Record>& a, long long key) {
    int s1, s2, s3;
    int i1 = binarySearch(a, key, s1);
    int i2 = interpolationSearch(a, key, s2);
    int i3 = exponentialSearch(a, key, s3);
    cout << "key " << key << ":\n";
    cout << "   binary        index " << setw(7) << i1 << "  steps " << s1 << "\n";
    cout << "   interpolation index " << setw(7) << i2 << "  steps " << s2 << "\n";
    cout << "   exponential   index " << setw(7) << i3 << "  steps " << s3 << "\n";
    if (i1 >= 0) cout << "   -> " << a[i1].name << ", gpa " << a[i1].gpa << "\n";
    else         cout << "   -> not in the dataset\n";
}

template <class F>
void bench(const string& name, const vector<Record>& a, const vector<long long>& keys, F search) {
    long long totalSteps = 0, sink = 0;
    for (long long k : keys) { int s; sink += search(a, k, s); totalSteps += s; }
    double best = 1e30;
    for (int rep = 0; rep < 3; ++rep) {
        auto t0 = chrono::steady_clock::now();
        for (long long k : keys) { int s; sink += search(a, k, s); }
        auto t1 = chrono::steady_clock::now();
        best = min(best, chrono::duration<double, nano>(t1 - t0).count() / keys.size());
    }
    cout << "   " << left << setw(14) << name << right << setw(8) << fixed << setprecision(2)
         << double(totalSteps) / keys.size() << " steps/search" << setw(9) << setprecision(1)
         << best << " ns/search" << (sink == -1 ? "!" : "") << "\n";
}

int main(int argc, char** argv) {
    auto data = makeDataset(150000);
    cout << "Dataset: " << data.size() << " records, ids " << data.front().id << " to " << data.back().id << "\n\n";

    if (argc > 1) { searchAll(data, stoll(argv[1])); return 0; }

    cout << "=== Demo searches ===\n";
    searchAll(data, data[0].id);          // first record
    searchAll(data, data[75000].id);      // middle
    searchAll(data, data[149999].id);     // last
    searchAll(data, 42);                  // not present

    mt19937 rng(7);
    uniform_int_distribution<int> any(0, (int)data.size() - 1), front(0, 99);
    vector<long long> randomKeys(1000000), frontKeys(1000000);
    for (auto& k : randomKeys) k = data[any(rng)].id;
    for (auto& k : frontKeys)  k = data[front(rng)].id;

    auto bin = [](const vector<Record>& a, long long k, int& s) { return binarySearch(a, k, s); };
    cout << "\n=== Benchmark A: 1,000,000 random keys ===\n";
    bench("binary", data, randomKeys, bin);
    bench("interpolation", data, randomKeys, interpolationSearch);
    bench("exponential", data, randomKeys, exponentialSearch);
    cout << "\n=== Benchmark B: 1,000,000 keys from the first 100 records ===\n";
    bench("binary", data, frontKeys, bin);
    bench("interpolation", data, frontKeys, interpolationSearch);
    bench("exponential", data, frontKeys, exponentialSearch);
    return 0;
}
