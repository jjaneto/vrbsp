//
// Created by José Joaquim on 2019-06-18.
//

#include <cstdio>
#include <iostream>
#include "Model.h"

int main() {

  std::chrono::time_point<std::chrono::system_clock> end, start;
  char file[30] = "dummy";
//
  Model *model;

  model = new Model(file, Model::type::linear);

  start = std::chrono::system_clock::now();
  model->solve();
  end = std::chrono::system_clock::now();
  puts("oie?");
  return 0;
}