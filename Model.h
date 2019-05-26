//
// Created by José Joaquim on 2019-05-19.
//

#ifndef VRBSP_MODEL_H
#define VRBSP_MODEL_H

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
#include <vector>

#elif
#include <bits/stdc++.h>
#endif

#include "gurobi_c++.h"
#include "Structures.h"

class Model {
public:

    enum type {
        nonlinear,
        linearBIGM,
        linear
    };

    Model(char file[], type formulation);

    ~Model();

    GRBEnv *env;
    GRBModel *model;
    GRBVar **vars;
    GRBLinExpr objectiveFunction;
    std::vector<GRBLinExpr *> constraints;

    int nDecisionVariables;
    int nConstraints;
    int nConnections;
    int nChannels, nChannels20MHz, nChannels40MHz, nChannels80MHz, nChannels160MHz;

    std::vector<std::vector<double>> interferenceMatrix;
    std::vector<std::vector<double>> distanceMatrix;
    std::vector<Coordenate> sendersCoord, recCoord;
    std::vector<Connection> connections;

    double dataRates[10][4], SINR[10][4];

    double time, alfa, noise, powerSender;

    void defineObjectiveFunction();

    void defineConstraints();

    void generateInterferenceDistanceMatrix();

    void createVariables();
};

#endif //VRBSP_MODEL_H

