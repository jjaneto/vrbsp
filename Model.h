//
// Created by José Joaquim on 2019-05-19.
//

#ifndef VRBSP_MODEL_H
#define VRBSP_MODEL_H

#endif //VRBSP_MODEL_H

#ifdef __APPLE__

#include <cstdio>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include <set>
#include <map>
#include <deque>
#include <iomanip>

#elif
#include <bits/stdc++.h>
#endif

#include "gurobi_c++.h"

class Model {
public:

    GRBEnv *env;
    GRBModel *model;
    GRBVar **vars;
    GRBLinExpr objectiveFunction;
    std::vector<GRBLinExpr *> constraints;

    int nDecisionVariables;
    int nConstraints;

    void defineObjectiveFunction();
    void defineConstraints();
};