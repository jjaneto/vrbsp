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

    int nDecisionVariables;
    int nConstraints;
    int nConnections;
    int nDataRates;
    int nChannels, nChannels20MHz, nChannels40MHz, nChannels80MHz, nChannels160MHz;

    GRBEnv *env;
    GRBModel *model;
    GRBVar **vars; //TODO: refactor this variable
    GRBVar x[250][250][50], y[250][250][4][10], z[250][250][45], I, IC;
    GRBLinExpr objectiveFunction;
    std::vector<GRBLinExpr *> constraints;

    type _type;

    std::vector<std::vector<double>> interferenceMatrix;
    std::vector<std::vector<double>> distanceMatrix;
    std::vector<Coordenate> sendersCoord, recCoord;
    std::vector<Connection> connections;

    double dataRates[10][4], SINR[10][4];

    double time, alfa, noise, powerSender;

    void defineObjectiveFunction();

    void defineConstraints();

    void generateInterferenceDistanceMatrix();

    void createDecisionVariables();

    void defineConstraintOne();

    void defineConstraintTwo();

    void defineConstraintThree();

    void defineConstraintFour();

    void defineConstraintFive();

    void defineConstraintSix();

    void defineConstraintSeven();

    void defineConstraintEight();

    void defineConstraintNine();

    void defineConstraintTen();

    void defineConstraintEleven();

    void defineConstraintTwelve();

    void defineConstraintThirteen();

};

#endif //VRBSP_MODEL_H

