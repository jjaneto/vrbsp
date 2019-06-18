//
// Created by José Joaquim on 2019-05-19.
//

#include "Model.h"

Model::Model(char *file, type formulation) {
  FILE *file1 = fopen(file, "r");

  if (file1 == nullptr) {
    fprintf(stderr, "Error opening file");
    exit(1);
  }

  _type = formulation;
  nChannels = 45, nChannels20MHz = 25, nChannels40MHz = 12, nChannels80MHz = 6, nChannels160MHz = 2;

  //TODO: change the dummy variables to its respective correct names

  double aux1;
  fscanf(file1, "%lf", &aux1);
  fscanf(file1, "%d %lf %lf %lf %lf %lf %lf %lf %lf", &nConnections, &time, &alfa, &noise, &powerSender, &aux1, &aux1,
         &aux1, &aux1);

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

  env = new GRBEnv();
  model = new GRBModel(*env);
  createDecisionVariables();
  defineObjectiveFunction();
  defineConstraints();
}

Model::~Model() {

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

void Model::createDecisionVariables() {

  //variavel x
  for (int i = 0; i < nConnections; i++) {
    //
    for (int j = 0; j < nConnections; j++) {
      //
      for (int c = 0; c < nChannels; c++) {
        std::string name = "var_X" + std::to_string(i) + "-" + std::to_string(j) + "-" + std::to_string(c);
        x[i][j][c] = model->addVar(0.0, 1.0, 0.0, GRB_INTEGER, name);
      }
    }
  }

  //variavel y
  for (int i = 0; i < nConnections; i++) {

    for (int j = 0; j < nConnections; j++) {

      for (int b = 0; b < 4; b++) {

        for (int d = 0; d < 10; d++) {

          y[i][j][b][d] = model->addVar(0.0, 1.0, 0.0, GRB_INTEGER, ""); //TODO: Put the right name

        }
      }
    }
  }

  //variavel z
  for (int i = 0; i < nConnections; i++) {

    for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {

        z[i][j][c] = model->addVar(0.0, 1.0, 0.0, GRB_INTEGER, ""); //TODO: Put the right name
      }
    }
  }

  //variavel I_{ij}
  for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {

      I[i][j] = model->addVar(0.0, 1.0, 0.0, GRB_INTEGER, ""); //TODO: Put the right name
    }
  }


  //Variavel I_{ij}^{c}
  for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {

        IC[i][j][c] = model->addVar(0.0, 1.0, 0.0, GRB_INTEGER, ""); //TODO: Put the right name
      }
    }
  }


}

void Model::defineConstraintOne() {

  /*for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {
      if (i != j) {
        GRBLinExpr expr1, expr2;

        //TODO: Implement Here.

        model->addConstr(expr1 + expr2 <= 1.0);
      }
    }
  }*/

  for (int i = 0; i < nConnections; i++) {

    for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {

        GRBLinExpr expr1, expr2;

        expr1 = x[i][j][c];
        expr2 = x[j][i][c];

        model->addConstr(expr1 + expr2 <= 1.0);
      }
    }
  }
}

void Model::defineConstraintTwo() {

  for (int b = 0; b < 4; b++) { //TODO: Instead of fixing b at 4, try to make it variable, as an extension of the model.

    if (b == 0) {
      nDataRates = 9;
    } else {
      nDataRates = 10;
    }

    for (int i = 0; i < nConnections; i++) {

      for (int j = 0; j < nConnections; j++) {
//        if (i == j)
//          continue;

        GRBLinExpr expr1, expr2;

        for (int s = 0; s < nDataRates; s++) {
          expr1 += y[i][j][b][s];
        }

        switch(b) {
          case 0:
            for (int c = 0; c < nDataRates; c++)
              expr2 += x[i][j][channels20MHz[c]];

            break;
          case 1:
            for (int c = 0; c < nDataRates; c++)
              expr2 += x[i][j][channels40MHz[c]];

            break;
          case 2:
            for (int c = 0; c < nDataRates; c++)
              expr2 += x[i][j][channels80MHz[c]];

            break;
          case 3:
            for (int c = 0; c < nDataRates; c++)
              expr2 += x[i][j][channels160MHz[c]];

            break;
        }


        model->addConstr(expr1 <= expr2);
      }
    }
  }

}

void Model::defineConstraintThree() {

  for (int i = 0; i < nConnections; i++) {

    for (int j = 0; j < nConnections; j++) {

      for (int c1 = 0; c1 < nChannels; c1++) {
        GRBLinExpr expr;
        for (int c2 = 0; c2 < nChannels; c2++) {

          if (overlap[c1][c2]) {
            expr += x[i][j][c1];
          }

        }
        model->addConstr(expr == z[i][j][c1]);
      }
    }
  }

}

void Model::defineConstraintFive() {

  for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {
        GRBLinExpr expr;

        expr = IC[i][j][c] - M_ij[i] * (1 - x[i][j][c]);

        model->addConstr(I[i][j] >= expr);
      }
    }
  }

}

void Model::defineConstraintSix() {

  for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {
        GRBLinExpr expr;

        expr = IC[i][j][c] + M_ij[i] * (1 - x[i][j][c]);

        model->addConstr(I[i][j] <= expr);
      }
    }
  }
}

void Model::defineConstraintSeven() {
  for (int i = 0; i < nConnections; i++) {
    for (int j = 0; j < nConnections; j++) {
      GRBLinExpr expr;

      for (int b = 0; b < 4; b++) {

        if (b == 0) {
          nDataRates = 9;
        } else {
          nDataRates = 10;
        }

        for (int s = 0; s < nDataRates; s++) {

          expr += ((connections[i].powerSR / SINR[s][b]) - noise) * y[i][j][b][s];
        }
      }
    }
  }
}

void Model::defineConstraints() {
  if (_type == nonlinear) {

  } else if (_type == linear) {

  } else {

  }
}

void Model::defineObjectiveFunction() {
  GRBLinExpr function;

  for (int i = 0; i < nConnections; i++) {

    for (int j = 0; j < nConnections; j++) {

      for (int b = 0; b < nChannels; b++) {

        for (int s = 0; s < nDataRates; s++) {

          function += ((i == j) ? dataRates[s][b] * y[i][j][b][s] : 0); //TODO: Check whether if it is [s][b] or [b][s]
        }
      }
    }
  }

  model->setObjective(function, GRB_MAXIMIZE);
}













