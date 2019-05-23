//
// Created by José Joaquim on 2019-05-19.
//

#include "Model.h"

void Model::defineConstraints() {

}

void Model::defineObjectiveFunction() {

}

Model::Model(char *file) {
  FILE *file1 = fopen(file, "r");

  if (file1 == nullptr) {
    fprintf(stderr, "Error opening file");
    exit(1);
  }

  nChannels = 45, nChannels20MHz = 25, nChannels40MHz = 12, nChannels80MHz = 6, nChannels160MHz = 2;

  //TODO: change the dummy variables to its respective correct names

  double aux1, aux2, aux3, aux4, aux5, aux6, aux7, aux8, aux9, aux10;
  fscanf(file1, "%lf", &aux1);
  fscanf(file1, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &aux2, &aux3, &aux4, &aux5, &aux6, &aux7, &aux8, &aux9, &aux10);

  //interferenceMatrix.assign();
  //distanceMatrix.assing();

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      fscanf(file1, "%lf", &dataRates[i][j]);
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      fscanf(file1, "%lf", &SINR[i][j]);
    }
  }

  for (int i = 0; i < nConnections; i++) {
    Coordenate coord;
    fprintf(file1, "%lf %lf", &coord.x, &coord.y);

    recCoord.emplace_back(coord);
  }

  for (int i = 0; i < nConnections; i++) {
    Coordenate coord;
    fprintf(file1, "%lf %lf", &coord.x, &coord.y);

    sendersCoord.emplace_back(coord);
  }

  generateInterferenceDistanceMatrix();

  //initBig_Mij();

}