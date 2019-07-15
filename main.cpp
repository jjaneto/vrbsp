//
// Created by José Joaquim on 2019-06-18.
//

#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include "Model.h"

int main() {

  using namespace std;
//  char file[100] = "./draft/CPLEX_BigM/src/instancias/U_8/U_8_1.txt";

  int l = 8;
  bool teste = true;
  std::string file = "./draft/CPLEX_BigM/src/instancias/U_" + to_string(l) + "/U_" + to_string(l) + "_";
  std::string extensao = ".txt";

  vector<string> lines;

  for (int i = 1; i <= 30; i++) {
    Model *model;
    std::string arquivo = file + std::to_string(i) + extensao;
    std::string saida = "results/";

    if (!teste) {
      saida += "L_" + to_string(l) + "/";
    } else {
      saida += "teste/L_" + to_string(l) + "/";
    }

    int result = mkdir(saida.c_str(), 0755);
    if (result < 0) {
//      printf("criar diretorio: %s | out = %d\n", saida.c_str(), result);
      if (errno != EEXIST) {
        exit(EXIT_FAILURE);
      }
    }

    model = new Model(arquivo, Model::type::linear_bigM, i, saida);

//    model->turnOffLogConsole(true);
    model->setLogToMyDefaultFile();
    model->solve();
    model->printResults();

    char out[100];
    sprintf(out, "%.3lf %.3lf %.3lf %.3lf\n", model->getObjVal(), model->getObjBound(), model->getMIPGap(),
            model->getRuntime());
    lines.push_back(string(out));

    delete model;
  }

  for (string s : lines) {
    cout << s;
  }


  return 0;
}