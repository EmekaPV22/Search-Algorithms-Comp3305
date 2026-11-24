// Binary search on 150,000 sorted records.   g++ -O2 -std=c++17 1_binary_search.cpp -o bs && ./bs
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

struct Record { long long id; string name; };

vector<Record> makeDataset(int n) {                 // sorted by id, ids spaced 1..9 apart
    mt19937 rng(1);
    uniform_int_distribution<int> gap(1, 9);
    vector<Record> v;
    long long id = 100000;
    for (int i = 0; i < n; ++i) { id += gap(rng); v.push_back({id, "Record_" + to_string(i)}); }
    return v;
}

// Returns index of key, or -1. 'steps' counts comparisons.
int binarySearch(const vector<Record>& a, long long key, int& steps) {
    int low = 0, high = (int)a.size() - 1;
    steps = 0;
    while (low <= high) {
        int mid = low + (high - low) / 2;           // avoids overflow of (low+high)/2
        ++steps;
        if (a[mid].id == key)      return mid;      // found
        else if (a[mid].id < key)  low = mid + 1;   // key is in the right half
        else                       high = mid - 1;  // key is in the left half
    }
    return -1;                                      // not found
}

int main() {
    auto data = makeDataset(150000);
    cout << "Records: " << data.size() << "\n";
    long long keys[] = {data[0].id, data[75000].id, data[149999].id, data[1234].id + 0, 5};
    for (long long k : keys) {
        int steps, idx = binarySearch(data, k, steps);
        cout << "key " << k << " -> " << (idx >= 0 ? "found at index " + to_string(idx) : "not found")
             << " in " << steps << " comparisons\n";
    }
}
