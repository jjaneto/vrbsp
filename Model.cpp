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

  double aux1;
  fscanf(file1, "%lf", &aux1);
  fscanf(file1, "%d %lf %lf %lf %lf %lf %lf %lf %lf", &nConnections, &time, &alfa, &noise, &powerSender, &aux1, &aux1, &aux1, &aux1);

  /*  //TODO: Check why this is necessary
  if (noise != 0) {
    noise = convertDBMToMW(noise);
  }
   */

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

  //TODO: Check why this is necessary
  //convertTableToMW(SINR, SINR, 10, 4);

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

void Model::generateInterferenceDistanceMatrix() {

  Connection auxConnection;
  Interference auxInterference;
  double coordX1, coordY1, coordX2, coordY2, xResult, yResult, result, inteferenceSenderReceptor;

  for (int i = 0; i < nConnections; i++) {

    coordX1 = recCoord[i].x;
    coordY1 = recCoord[i].y;

    coordX2 = sendersCoord[i].x;
    coordY2 = sendersCoord[i].y;


    xResult = (coordX1 - coordX2) * (coordX1 - coordX2);
    yResult = (coordY1 - coordY2) * (coordY1 - coordY2);

    result = sqrt(xResult + yResult);

    auxConnection.id = i;
    auxConnection.distanceSenderReceptor = result;

    auxConnection.powerSR = powerSender / pow(result, alfa);

    auxConnection.totalInterference = 0.0;

    connections.emplace_back(auxConnection);

    for (int j = 0; j < nConnections; j++) {
      if (i != j) {

        auxInterference.idReceptor = i;
        auxInterference.idSender = j;

        coordX2 = sendersCoord[j].x;
        coordY2 = recCoord[j].y;

        if (coordX1 == coordX2 && coordY1 == coordY2) { //TODO: Do the comparison in the right way (with epsilon)
          result = 0.0;

          auxInterference.distanceConnections = result;

          auxInterference.valueInterference = 1000000000;

          interferenceMatrix[i][j] = auxInterference.valueInterference;

          distanceMatrix[i][j] = result;

        } else {

          xResult = (coordX1 - coordX2) * (coordX1 - coordX2);
          yResult = (coordY1 - coordY2) * (coordY1 - coordY2);

          result = sqrt(xResult + yResult);


          //Interference between the Receptor[i] and the Sender[j] (Pii/(distance_ij^alfa))
          //OBS: See the referent constraint in the mathematical model
          auxInterference.distanceConnections = result;

          auxInterference.valueInterference = powerSender / pow(result, alfa);

          interferenceMatrix[i][j] = auxInterference.valueInterference;

          distanceMatrix[i][j] = result;

        }

      } else {
        auxInterference.idReceptor = i;
        auxInterference.idSender = j;
        auxInterference.distanceConnections = auxConnection.distanceSenderReceptor;//Distance Between Sender[i] and Receptor[j]
        auxInterference.valueInterference = 0;

        interferenceMatrix[i][j] = 0.0;

        distanceMatrix[i][j] = auxConnection.distanceSenderReceptor;
      }
    }
  }

  //TODO: Why is this necessary? Or, actually, is this necessary?
  //recCoord.clear();
  //sendersCoord.clear();
}













