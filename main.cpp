//
// Created by José Joaquim on 2019-06-18.
//

#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include "Model.h"

struct instance {
    int L;
    int code;
    double timeLimit;
    std::string area;
    Model::type tp;
};

void printLines(std::vector<std::string> lines) {
  for (std::string s : lines) {
    printf("%s\n", s.c_str());
  }
}

std::string getTypeString(Model::type tp) {
  std::string ret = "-1";
  if (tp == Model::type::bigM) {
    ret = "bigM";
  } else if (tp == Model::type::W) {
    ret = "W";
  } else if (tp == Model::type::nonLinear) {
    ret = "nonLinear";
  } else if (tp == Model::type::bigM2) {
    ret = "bigM2";
  } else if (tp == Model::type::W2) {
    ret = "W2";
  }
  return ret;
}

int main(int argc, char **argv) {

  using namespace std;
  FILE *openFile = fopen(argv[1], "r");

  if (openFile == NULL) {
    fprintf(stderr, "Error during file opening. Closing program...\n");
    exit(-1);
  }

  instance inst;
  //
  fscanf(openFile, "%d", &inst.L);
  //
  char area[15];
  fscanf(openFile, "%s", area);
  if (!strcmp(area, "D250x250") || !strcmp(area, "D10000x10000")) {
    inst.area = string(area);
  } else {
    fprintf(stderr, "Error during area reading (I've read %s). Closing program...\n", area);
    exit(-1);
  }
  //
  char type[15];
  fscanf(openFile, "%s", type);
  if (strcmp(type, "W") == 0) {
    inst.tp = Model::type::W;
  } else if (strcmp(type, "bigM") == 0) {
    inst.tp = Model::type::bigM;
  } else if (strcmp(type, "nonLinear") == 0) {
    inst.tp = Model::type::nonLinear;
  } else if (strcmp(type, "W2") == 0) {
    inst.tp = Model::type::W2;
  } else if (strcmp(type, "bigM2") == 0) {
    inst.tp = Model::type::bigM2;
  }else {
    fprintf(stderr, "Error during type model reading (I've read %s). Closing program...\n", type);
    exit(-1);
  }
  //
  fscanf(openFile, "%lf", &inst.timeLimit);
  //
  int run;
  fscanf(openFile, "%d", &run);

  vector<string> lines;
//  for (int i = begin; i <= end; i++) {

    std::string file = "instancias/" + inst.area +"/U_" + to_string(inst.L) + "/U_" + to_string(inst.L) + "_";
    std::string extensao = ".txt";

    std::string entrada = file + std::to_string(run) + extensao;
    std::string saida = "results/" + inst.area + "/" + getTypeString(inst.tp) + "/L_" + to_string(inst.L) + "/";

    Model *model = new Model(entrada, inst.tp, run, saida);

    model->turnOffLogConsole(true);
    model->setLogToMyDefaultFile();
    if (inst.timeLimit != -1.0) {
      model->setTimeLimit(inst.timeLimit);
    }

    printf("Otimizando entrada %s\n", entrada.c_str());
    model->solve();
    model->printResults();
    string w = saida + "outSol_" + to_string(run) + ".sol";
    model->writeGurobiOutSolution(w);

//    char out[100];
//    sprintf(out, "%.3lf %.3lf %.3lf %.3lf\n", model->getObjVal(), model->getObjBound(), model->getMIPGap(),
//            model->getRuntime());
//    lines.push_back(string(out));

    delete model;
//  }

  return 0;
}