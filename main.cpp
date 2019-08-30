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

#ifdef TESTE
  puts("adas");
#endif

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
  fscanf(openFile, "%d", &inst.code);
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

  int executionTimes;
  fscanf(openFile, "%d", &executionTimes);

  if (executionTimes > 30) {
    puts("Read more than 30 executions. Setting execution times to 30...");
    executionTimes = 30;
  }

  vector<string> lines;
  for (int i = 0; i < executionTimes; i++) {
    int INSTANCE_TO_BE_EXECUTED = inst.code + i;

    std::string file = "instancias/U_" + to_string(inst.L) + "/U_" + to_string(inst.L) + "_";
    std::string extensao = ".txt";

    std::string entrada = file + std::to_string(INSTANCE_TO_BE_EXECUTED) + extensao;
    std::string saida = "results/" + getTypeString(inst.tp) + "/L_" + to_string(inst.L) + "/";

    Model *model = new Model(entrada, inst.tp, INSTANCE_TO_BE_EXECUTED, saida);

    model->turnOffLogConsole(true);
    model->setLogToMyDefaultFile();
    if (inst.timeLimit != -1.0) {
      model->setTimeLimit(inst.timeLimit);
    }

    printf("Otimizando entrada %s\n", entrada.c_str());
    model->solve();
    model->printResults();
    string w = saida + "outSol_" + to_string(INSTANCE_TO_BE_EXECUTED) + ".sol";
    model->writeGurobiOutSolution(w);

//    char out[100];
//    sprintf(out, "%.3lf %.3lf %.3lf %.3lf\n", model->getObjVal(), model->getObjBound(), model->getMIPGap(),
//            model->getRuntime());
//    lines.push_back(string(out));

    delete model;
  }

  return 0;
}