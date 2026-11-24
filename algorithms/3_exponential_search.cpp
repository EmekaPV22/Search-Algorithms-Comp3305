// Exponential search on 150,000 sorted records.   g++ -O2 -std=c++17 3_exponential_search.cpp -o es && ./es
#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

struct Record { long long id; string name; };

vector<Record> makeDataset(int n) {
    mt19937 rng(1);
    uniform_int_distribution<int> gap(1, 9);
    vector<Record> v;
    long long id = 100000;
    for (int i = 0; i < n; ++i) { id += gap(rng); v.push_back({id, "Record_" + to_string(i)}); }
    return v;
}

// binary search restricted to a[low..high]
int binarySearchRange(const vector<Record>& a, long long key, int low, int high, int& steps) {
    while (low <= high) {
        int mid = low + (high - low) / 2;
        ++steps;
        if (a[mid].id == key)      return mid;
        else if (a[mid].id < key)  low = mid + 1;
        else                       high = mid - 1;
    }
    return -1;
}

int exponentialSearch(const vector<Record>& a, long long key, int& steps) {
    int n = (int)a.size();
    steps = 0;
    if (n == 0) return -1;
    ++steps;
    if (a[0].id == key) return 0;                   // check the first element
    int bound = 1;                                  // gallop: 1, 2, 4, 8, 16, ...
    while (bound < n && a[bound].id < key) { ++steps; bound *= 2; }
    // the key (if present) is in [bound/2, min(bound, n-1)]
    return binarySearchRange(a, key, bound / 2, min(bound, n - 1), steps);
}

int main() {
    auto data = makeDataset(150000);
    cout << "Records: " << data.size() << "\n";
    long long keys[] = {data[0].id, data[10].id, data[75000].id, data[149999].id, 5};
    for (long long k : keys) {
        int steps, idx = exponentialSearch(data, k, steps);
        cout << "key " << k << " -> " << (idx >= 0 ? "found at index " + to_string(idx) : "not found")
             << " in " << steps << " comparisons\n";
    }
}
