//
// Created by José Joaquim on 2019-06-18.
//

#include <cstdio>
#include <iostream>
#include "Model.h"

int main() {

  std::chrono::time_point<std::chrono::system_clock> end, start;
  char file[100] = "./draft/CPLEX_BigM/src/instancias/U_8/U_8_1.txt";
  Model *model;

  model = new Model(file, Model::type::linear);

//  start = std::chrono::system_clock::now();
  model->solve();
//  end = std::chrono::system_clock::now();

  int status = model->getStatus();

  if (status == GRB_OPTIMAL) {
    //Do something
  }
  return 0;
}