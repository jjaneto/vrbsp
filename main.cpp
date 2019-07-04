//
// Created by José Joaquim on 2019-06-18.
//

#include <cstdio>
#include <iostream>
#include "Model.h"

int main() {

  using namespace std;
  chrono::time_point<std::chrono::system_clock> end, start;
//  char file[100] = "./draft/CPLEX_BigM/src/instancias/U_8/U_8_1.txt";

  int l = 128;
  std::string file = "./draft/CPLEX_BigM/src/instancias/U_" + to_string(l) + "/U_" + to_string(l) + "_";
  std::string extensao = ".txt";


  for (int i = 1; i <= 30; i++) {
    Model *model;
    std::string arquivo = file + std::to_string(i) + extensao;
    std::string saida = "results/no-heuristic/L_" + to_string(l) + "/";
    printf("arquivo eh %s\n", arquivo.c_str());
    model = new Model(arquivo, Model::type::linear, i, saida);

    //  start = std::chrono::system_clock::now();
    model->solve();
    model->printResults();

    int status = model->getStatus();
    if (status == GRB_OPTIMAL) {
      //Do something
      puts("Optimal found!");
    }


    delete model;
  }


  return 0;
}