// Copyright (c) 2018, The Hebrew University of Jerusalem (HUJI, A. Barak)
// See LICENSE.txt for license details
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


/*This file computes the load imbalance of many hash functions : jch, cyclic, range, snake, and ,rotation */

/*
GAP Benchmark Suite
Kernel: Connected Components (CC)
Authors: Michael Sutton, Scott Beamer
Will return comp array labelling each vertex with a connected component ID
This CC implementation makes use of the Afforest subgraph sampling algorithm [1],
which restructures and extends the Shiloach-Vishkin algorithm [2].
[1] Michael Sutton, Tal Ben-Nun, and Amnon Barak. "Optimizing Parallel 
    Graph Connectivity Computation via Subgraph Sampling" Symposium on 
    Parallel and Distributed Processing, IPDPS 2018.
[2] Yossi Shiloach and Uzi Vishkin. "An o(logn) parallel connectivity algorithm"
    Journal of Algorithms, 3(1):57–67, 1982.
*/
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
int cyc_hash(int numP, int v) {
  return (v % numP);
}
int range_hash(int numVertices, int numP, int v) {
  //numVertices is the total number of vertices of Graph g
  int vertP = (numVertices+(numP-1))/numP;
  return (v/vertP);
}
int snake_hash(int numP, int v) {
  return ((numP - (v % numP))-1);
}
int rotation_hash(int numP, int v) {
    int rotation_offset = v / numP;
    return (v + rotation_offset) % numP;
}
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
    // create the files for each hash
    std::fstream cyc_file("cyc_outfile.json", std::ios::out | std::ios::binary);
    std::fstream range_file("range_outfile.json", std::ios::out | std::ios::binary);
    std::fstream jch_file("jch_outfile.json", std::ios::out | std::ios::binary);  
    std::fstream rotation_file("rotation_outfile.json", std::ios::out | std::ios::binary);  
    std::fstream snake_file("snake_outfile.json", std::ios::out | std::ios::binary);
   
    // create JSON arrays to store data
    JSON partitions = JSON::Make(JSON::Class::Array);
    JSON cyc_imbalances = JSON::Make(JSON::Class::Array);
    JSON range_imbalances = JSON::Make(JSON::Class::Array);
    JSON jch_imbalances = JSON::Make(JSON::Class::Array); 
    JSON rotation_imbalances = JSON::Make(JSON::Class::Array); 
    JSON snake_imbalances = JSON::Make(JSON::Class::Array); 
    
    // initalize variables for hash index
    int destPC = 0;
    int destPR = 0;
    int destJCH = 0;  
    int destRotation = 0;
    int destSnake = 0;
    int numVert = g.num_nodes();
    
    for (int numP = 1; numP <= maxP; numP*=2) {
        int JCH_TOTAL = 0;
        vector<int64_t> totalsC(numP, 0);
        vector<int64_t> totalsR(numP, 0);
        vector<int64_t> totalsJCH(numP, 0);  
        vector<int64_t> totalsRotation(numP, 0);
        vector<int64_t> totalsSnake(numP, 0);

        for (auto x : g.vertices()) {
          // cyclic hash function
          destPC = cyc_hash(numP, x);

          // range hash function
          destPR = range_hash(numVert, numP, x);

          // JCH hash function
          destJCH = jch_hash(x, numP);

          // rotation hash function
          destRotation = rotation_hash(numP,x);
          
          //snake hash function
          destSnake = snake_hash(numP, x);
          
          // store the out degree in the corresponding array
          totalsC[destPC] += g.out_degree(x);
          totalsR[destPR] += g.out_degree(x);
          totalsJCH[destJCH] += g.out_degree(x);  
          totalsRotation[destRotation] += g.out_degree(x);
          totalsSnake[destSnake] += g.out_degree(x);
        }
        // compute the imbalance of each hash
        double comp_C = compute_imbalance(totalsC);
        double comp_R = compute_imbalance(totalsR);
        double comp_JCH = compute_imbalance(totalsJCH);  
        double comp_Rotation = compute_imbalance(totalsRotation);
        double comp_Snake = compute_imbalance(totalsSnake);
        
        
        partitions.append(numP);
        cyc_imbalances.append(comp_C);
        range_imbalances.append(comp_R);
        jch_imbalances.append(comp_JCH); 
        rotation_imbalances.append(comp_Rotation);
        snake_imbalances.append(comp_Snake);


    }
    JSON cyc_data = generate_json_data("cyc", partitions, cyc_imbalances);
    JSON range_data = generate_json_data("range", partitions, range_imbalances);
    JSON jch_data = generate_json_data("jch", partitions, jch_imbalances);  
    JSON Rotation_data = generate_json_data("rotation", partitions, rotation_imbalances);
    JSON snake_data = generate_json_data("snake", partitions, snake_imbalances);
    
    // store all the data in corresponding files
    cyc_file << cyc_data;
    range_file << range_data;
    jch_file << jch_data; 
    rotation_file << Rotation_data;
    snake_file << snake_data;

    cyc_file.close();
    range_file.close();
    jch_file.close(); 
    rotation_file.close();
    snake_file.close();
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