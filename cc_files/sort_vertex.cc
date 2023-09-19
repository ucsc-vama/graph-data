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
// Where to store the data
  std::fstream sort_file("sort_ver_outfile.json", std::ios::out | std::ios::binary);
  // create a json array to store the number of median
  JSON sort_vec = JSON::Make(JSON::Class::Array);
  
  // declare a new vector to store the total num of out degree in each partition
  vector<int64_t> total_part(maxP, 0);
  // declare a new vector to copy vertices
  vector<int64_t> od_vec;

  // copy the out degrees from the original Graph
  for (auto i : g.vertices()){
    od_vec.push_back(g.out_degree(i));
  }
  // sort the od vector
  std::sort(od_vec.begin(), od_vec.end(), greater<NodeID>());

  // go through the vertices and find the one with least out degree
  for (NodeID d: od_vec) {
    // find the partition with the minimum out degree
    auto it = std::min_element(std::begin(total_part), std::end(total_part));
    int min_index =  std::distance(std::begin(total_part), it);
    // assign the vertex to the partition
    total_part[min_index] += 1;
  }

  for (auto i: total_part) {
    sort_vec.append(i);
  }
  // store the data in file
  JSON sort_data = generate_json_data("sort", maxP, sort_vec);
  sort_file << sort_data;
  sort_file.close(); 
}

int main(int argc, char* argv[]) {
  // set the number of partition
  int maxP = 1024;
  CLApp cli(argc, argv, "connected-components-afforest");
  if (!cli.ParseArgs())
    return -1;
  Builder b(cli);
  Graph g = b.MakeGraph();
  find_imbalance(g, maxP);
  return 0;
}