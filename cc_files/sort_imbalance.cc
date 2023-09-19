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

/* This file computes the load imbalance of sort hash*/
 using namespace std;
double compute_imbalance(vector<int64_t> totals) {
  int64_t max_ = 0;
  double total_ = 0;
  for (auto x : totals) {
    max_ = max(max_,x);
    total_ += x;
  }
  double average_ = total_/totals.size();
  return max_/average_;
}
JSON generate_json_data(const std::string &hash_name, const JSON &partitions, const JSON &imbalances) {
    JSON all_data = JSON::Make(JSON::Class::Object);
    all_data["graph"] = "kron";
    all_data["hash"] = hash_name;
    all_data["partition"] = partitions;
    all_data["load imbalance"] = imbalances;
    return all_data;
}
void find_imbalance(const Graph &g, int maxP) {
  std::fstream sort_file("sort_outfile.json", std::ios::out | std::ios::binary);
  JSON partitions = JSON::Make(JSON::Class::Array);
  JSON sort_imbalances = JSON::Make(JSON::Class::Array); 
  
  for (int numP = 1; numP <= maxP; numP*=2) {
    // declare a new vector to store the total num of out degree in each partition
    vector<int64_t> total_part(numP, 0);
    // declare a new vector to copy vertices
    vector<int64_t> od_vec;
    // add another vector how many out degree in each partitions

    // copy the vertices
    for (auto i : g.vertices()){
      od_vec.push_back(g.out_degree(i));
    }
    // sort the od vector
    std::sort(od_vec.begin(), od_vec.end(), greater<NodeID>());
    
    // while there exist vectices, find the one with minimum od
    for (NodeID d: od_vec) {
      // find the partition with the minimum out degree
      auto it = std::min_element(std::begin(total_part), std::end(total_part));
      int min_index =  std::distance(std::begin(total_part), it);
      // assign the vertex to the partition
      total_part[min_index] += d;
    }
    for (int i = 0; i < total_part.size(); i++) {
      partitions.append(i);
      sort_imbalances.append(total_part[i]);
    }
    double comp_Sort = compute_imbalance(total_part);
}
  JSON sort_data = generate_json_data("sort", partitions, sort_imbalances);
  sort_file << sort_data;
  sort_file.close(); 
}
int main(int argc, char* argv[]) {
  int numP = 8192;
  CLApp cli(argc, argv, "connected-components-afforest");
  if (!cli.ParseArgs())
    return -1;
  Builder b(cli);
  Graph g = b.MakeGraph();
  find_imbalance(g, numP);
  return 0;
}
