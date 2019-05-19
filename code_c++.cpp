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

using ii = pair<int, int>;

const int MAXNODES = 1000;

// vector<int> GRAPH[MAXNODES];
vector<ii> edges;
vector<int> bandwidths;
int MCSIdentifiers[10];
int numberOfConstraints, numberOfVariables;

GRBEnv *env;
GRBModel *model;
GRBVar **vars;
GRBLinExpr *objective;
vector<GRBLinExpr *> constraints;

void readInstance() {

}

void createModel() {
  env = new GRBEnv();
  model = new GRBModel(*env);
  vars = new GRBVar *[numberOfVariables];

  for (int i = 0; i < numberOfVariables; i += 1) {

  }
}

void defineObjective() {
  *objective = 0.0;
  int node = -1;

}

void defineConstraints() {
  //TODO: constraints 5, 6, 9, 10, 11, 15, 16, 17
  constraints.resize(numberOfConstraints);

  GRBLinExpr constr1_1, constr1_2;
  for (const ii &edge : edges) {

  }
}

int main(int argc, char **argv) {
  readInstance();
  return 0;
}
