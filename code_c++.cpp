#ifdef __APPLE__
#include <cstdio>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include <set>
#include <map>
#include <deque>
#elif
#include <bits/stdc++.h>
#endif

#include "gurobi_c++.h"

using namespace std;

const int MAXNODES = 1000;

vector<int> GRAPH[MAXNODES];
vector<int> bandwidths;
int MCSIdentifiers[10];
int numberOfConstraints;

GRBEnv *env;
GRBModel *model;
GRBVar *vars;
GRBLinExpr *objective;
vector<GRBLinExpr *> constraints;

void readInstance() {
  
}

void defineObjective() {
  *objective = 0.0;
  int node = -1;
  for (const int x : GRAPH[node]) {
    
  }
}

void defineConstraints() {
  //TODO: constraints 5, 6, 9, 10, 11, 15, 16, 17
  constraints.resize(numberOfConstraints);

  for (GRBLinExpr *c : constraints) {
    
  }  
}

int main(int argc, char **argv) {
  
  return 0;
}
