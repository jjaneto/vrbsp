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

  int l = 16;
  std::string file = "./draft/CPLEX_BigM/src/instancias/U_" + to_string(l) + "/U_" + to_string(l) + "_";
  std::string extensao = ".txt";

  vector<string> lines;

  for (int i = 1; i <= 30; i++) {
    Model *model;
    std::string arquivo = file + std::to_string(i) + extensao;
    std::string saida = "results/no-heuristic/L_" + to_string(l) + "/";
//    printf("arquivo eh %s\n", arquivo.c_str());
    model = new Model(arquivo, Model::type::linearV1, i, saida);

    //  start = std::chrono::system_clock::now();
    model->solve();
//    model->printResults();

//    int status = model->getStatus();
//    if (status == GRB_OPTIMAL) {
//      puts("Optimal found!");
//    }

    char out[100];
    sprintf(out, "%.3lf %.3lf %.3lf %.3lf\n", model->getObjVal(), model->getObjBound(), model->getMIPGap(), model->getRuntime());
    lines.push_back(string(out));


    delete model;
  }

  for (string s : lines) {
    cout << s << '\n';
  }


  return 0;
}