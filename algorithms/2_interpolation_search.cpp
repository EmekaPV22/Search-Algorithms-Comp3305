// Interpolation search on 150,000 sorted records.   g++ -O2 -std=c++17 2_interpolation_search.cpp -o is && ./is
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

int interpolationSearch(const vector<Record>& a, long long key, int& steps) {
    int low = 0, high = (int)a.size() - 1;
    steps = 0;
    // the key can only be inside [a[low], a[high]]
    while (low <= high && key >= a[low].id && key <= a[high].id) {
        if (a[low].id == a[high].id) {              // all remaining values equal: avoid divide by 0
            ++steps;
            return a[low].id == key ? low : -1;
        }
        // estimate the position from the key's value
        double fraction = double(key - a[low].id) / double(a[high].id - a[low].id);
        int pos = low + int(fraction * (high - low));
        ++steps;
        if (a[pos].id == key)      return pos;
        else if (a[pos].id < key)  low = pos + 1;
        else                       high = pos - 1;
    }
    return -1;
}

int main() {
    auto data = makeDataset(150000);
    cout << "Records: " << data.size() << "\n";
    long long keys[] = {data[0].id, data[75000].id, data[149999].id, data[1234].id, 5};
    for (long long k : keys) {
        int steps, idx = interpolationSearch(data, k, steps);
        cout << "key " << k << " -> " << (idx >= 0 ? "found at index " + to_string(idx) : "not found")
             << " in " << steps << " probes\n";
    }
}
