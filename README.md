# Optimized Search Techniques: Binary, Interpolation and Exponential Search

COMP 3305 project. Three search algorithms for sorted data, implemented in C++17 and compared on a dataset of 150,000+ records.

| Algorithm | Average time | Worst case | Best when |
|---|---|---|---|
| Binary search | O(log n) | O(log n) | Any sorted data |
| Interpolation search | O(log log n) on evenly spread keys | O(n) | Evenly spread numeric keys |
| Exponential search | O(log i), i = key position | O(log n) | Key near the front, or unknown size |

## Files

- `search_all.cpp` – all three algorithms in one program (150,000 records, demo + benchmark)
- `algorithms/` – each algorithm as its own program (`1_binary_search.cpp`, `2_interpolation_search.cpp`, `3_exponential_search.cpp`)
- `search_benchmark.cpp` – larger benchmark: 200,000 records, evenly spread and skewed keys, scaling from 100K to 2M records
- `COMP3305_search_demo.ipynb` – Google Colab notebook that compiles and runs the benchmark online

## Build and run

```bash
g++ -O2 -std=c++17 search_all.cpp -o search_all
./search_all            # demo searches + benchmark
./search_all 475681     # search one specific id with all three algorithms
```

## Run online

Open `COMP3305_search_demo.ipynb` in [Google Colab](https://colab.research.google.com/) (File > Upload notebook, or open it from this repository) and choose Runtime > Run all.

## Sample results (1,000,000 random searches, 150,000 records)

| Algorithm | Steps per search | Time per search |
|---|---|---|
| Binary | 16.3 | 171 ns |
| Interpolation | 3.4 | 87 ns |
| Exponential | 31.2 | 199 ns |

Interpolation search collapses on heavily skewed keys (about 500x slower than binary in `search_benchmark.cpp`), so know your data's distribution before choosing.
