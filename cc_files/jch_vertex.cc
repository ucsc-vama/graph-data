#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "benchmark.h"
#include "bitmap.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "pvector.h"
#include "timer.h"
#include "json.hpp"
using json::JSON;

/* This file calculates the number of vertices distributed to each partition using the jch hash */

 using namespace std;
 int32_t jch_hash(uint64_t key, int32_t num_buckets) {
    int64_t b = -1, j = 0;
    while (j < num_buckets) {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
    }
    return b;
}
// this function computes the median of a vector
double compute_median(vector<int64_t> totals) {
  int64_t size_ = totals.size();
  if (size_ == 0)
  {
    cout << "ERROR: size less than 1" << endl;
    return 0;  // Undefined, really.
  }
  sort(totals.begin(), totals.end());
  if (size_ % 2 == 0) {
    return (totals[size_ / 2 - 1] + totals[size_ / 2]) / 2;
  }
  else {
    return totals[size_ / 2];
  }
}
// this function puts data in a json format
JSON generate_json_data(const std::string &hash_name, int &partitions, const JSON &out) {
    JSON all_data = JSON::Make(JSON::Class::Object);
    all_data["graph"] = "kron";
    all_data["hash"] = hash_name;
    all_data["partition"] = partitions;
    all_data["num of vertices array"] = out;
    return all_data;
}
void find_imbalance(const Graph &g, int maxP) {
    // create the files for each hash
    std::fstream jch_file("jch_ver_outfile.json", std::ios::out | std::ios::binary);  
    // create JSON arrays to store the median of each partition
    JSON vertex_vec = JSON::Make(JSON::Class::Array); 
    // initalize variables for hash index
    int destJCH = 0;
    int JCH_TOTAL = 0;
    vector<int64_t> totalsJCH(maxP, 0);
    // add a 2d vector how many out degree in each partitions
    vector<vector<int64_t>> my_vect(maxP);

    // run graph with the hash
    for (auto x : g.vertices()) {
        // JCH hash function
        destJCH = jch_hash(x, maxP);
        // keep count of how many vertices in each partitions
        totalsJCH[destJCH] += 1;
    }

    for (auto i : totalsJCH){
        vertex_vec.append(i);
    }
  // store the data in file
  JSON sort_data = generate_json_data("jch", maxP, vertex_vec);
  jch_file << sort_data;
  jch_file.close(); 
}

int main(int argc, char* argv[]) {
  // set the number of partition
  int numP = 1024;
  CLApp cli(argc, argv, "connected-components-afforest");
  if (!cli.ParseArgs())
    return -1;
  Builder b(cli);
  Graph g = b.MakeGraph();
  find_imbalance(g, numP);
  return 0;
}