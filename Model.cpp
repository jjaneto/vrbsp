//
// Created by José Joaquim on 2019-05-19.
//

#include "Model.h"

Model::Model(std::string file, type formulation, int number, std::string outputFile) {
  FILE *file_ = fopen(file.c_str(), "r");

  this->outputFile = outputFile;
  inst = number;

  if (file_ == nullptr) {
    fprintf(stderr, "Error opening file");
    exit(1);
  }

  dataRates.assign(10, std::vector<double>(4, 0));

  SINR.assign(10, std::vector<double>(4, 0));

  _type = formulation;
  nChannels = 45, nChannels20MHz = 25, nChannels40MHz = 12, nChannels80MHz = 6, nChannels160MHz = 2;

  //TODO: change the dummy variables to its respective correct names

  double aux1;
  fscanf(file_, "%lf", &aux1);
  fscanf(file_, "%d %lf %lf %lf %lf %lf %lf %lf %lf", &nConnections, &time, &alfa, &noise, &powerSender, &aux1, &aux1,
         &aux1, &aux1);

  //TODO: Check why this is necessary
  if (noise != 0) {
    noise = convertDBMToMW(noise);
  }


  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      fscanf(file_, "%lf", &dataRates[i][j]);
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      fscanf(file_, "%lf", &SINR[i][j]);
    }
  }

  //TODO: Check why this is necessary
  convertTableToMW(SINR, SINR, 10, 4);

  for (int i = 0; i < nConnections; i++) {
    Coordenate coord;
    fscanf(file_, "%lf %lf", &coord.x, &coord.y);

    recCoord.emplace_back(coord);
  }

  for (int i = 0; i < nConnections; i++) {
    Coordenate coord;
    fscanf(file_, "%lf %lf", &coord.x, &coord.y);

    sendersCoord.emplace_back(coord);
  }

  interferenceMatrix.assign(nConnections, std::vector<double>(nConnections, 0));
  distanceMatrix.assign(nConnections, std::vector<double>(nConnections, 0));

  generateInterferenceDistanceMatrix();
//
  createBigM();
//
  env = new GRBEnv();
  model = new GRBModel(*env);

  createDecisionVariables();
  defineConstraints();
  defineObjectiveFunction();

//  model->set(GRB_StringParam_LogFile, outputFile + "gurobiOUT_" + std::to_string(number) + ".txt");
//  model->set(GRB_DoubleParam_Heuristics, 0.0);
}

Model::~Model() {

}

void Model::generateInterferenceDistanceMatrix() { //TODO: Double check this function in order to remove erros

  Connection auxConnection;
  Interference auxInterference;
  double coordX1, coordY1, coordX2, coordY2, xResult, yResult, result;

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

}

void Model::createDecisionVariables() {

  char varName[50];
  //variavel x
  for (int i = 0; i < nConnections; i++) {
    //
    for (int j = 0; j < nConnections; j++) {
      //
      for (int c = 0; c < nChannels; c++) {

        sprintf(varName, "X_[%d][%d][%d]", i, j, c);
        x[j][c] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
      }
    }
  }

  //variavel y
//  for (int i = 0; i < nConnections; i++) {

  for (int j = 0; j < nConnections; j++) {

    for (int b = 0; b < 4; b++) {

      for (int d = 0; d < 10; d++) {

        sprintf(varName, "Y_[%d][%d][%d][%d]", j, j, b, d);
        y[j][b][d] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
      }
    }
  }
//  }

  //variavel z
//  for (int i = 0; i < nConnections; i++) {

  for (int j = 0; j < nConnections; j++) {

    for (int c = 0; c < nChannels; c++) {

      sprintf(varName, "Z_[%d][%d][%d]", j, j, c);
      z[j][c] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
    }
  }
//  }

  //variavel I_{ij}
  for (int i = 0; i < nConnections; i++) {
//    for (int j = 0; j < nConnections; j++) {

    sprintf(varName, "I_[%d][%d]", i, i);
    I[i] = model->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, varName);
//    }
  }


  if (_type == linear_bigM) {
    //Variavel I_{ij}^{c}
    for (int i = 0; i < nConnections; i++) {
//      for (int j = 0; j < nConnections; j++) {

      for (int c = 0; c < nChannels; c++) {

        sprintf(varName, "IC_[%d][%d][%d]", i, i, c);
        IC[i][c] = model->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, varName);
      }
//      }
    }
  } else if (_type == linearV1) {
    try {
//      for (int i = 0; i < nConnections; i++) {
      for (int j = 0; j < nConnections; j++) {

        for (int u = 0; u < nConnections; u++) {
          for (int v = 0; v < nConnections; v++) {
            sprintf(varName, "W_[%d][%d][%d][%d]", j, j, u, v);
            w[j][u] = model->addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, varName);
          }
        }
      }
//      }
    } catch (GRBException ex) {
      std::cout << "epa " + ex.getMessage() << "\n";
    }
  }


}

void Model::defineConstraintOne() {

  for (int i = 0; i < nConnections; i++) {

    GRBLinExpr expr1, expr2;

    for (int c = 0; c < nChannels; c++) {
      expr1 += x[i][c];
    }
    model->addConstr(expr1 <= 1.0);
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

      GRBLinExpr expr1, expr2;

      for (int s = 0; s < nDataRates; s++) {
        expr1 += y[i][b][s];
      }

      switch (b) {
        case 0:
          for (int c = 0; c < nChannels20MHz; c++)
            expr2 += x[i][channels20MHz[c]];

          break;
        case 1:
          for (int c = 0; c < nChannels40MHz; c++)
            expr2 += x[i][channels40MHz[c]];

          break;
        case 2:
          for (int c = 0; c < nChannels80MHz; c++)
            expr2 += x[i][channels80MHz[c]];

          break;
        case 3:
          for (int c = 0; c < nChannels160MHz; c++)
            expr2 += x[i][channels160MHz[c]];

          break;
      }

      model->addConstr(expr1 <= expr2);
    }
  }

}

void Model::defineConstraintThree() {

  for (int i = 0; i < nConnections; i++) {
    int j = i;

    for (int c1 = 0; c1 < nChannels; c1++) {
      GRBLinExpr expr;
      for (int c2 = 0; c2 < nChannels; c2++) {

        if (overlap[c1][c2]) {
          expr += x[j][c2];
        }

      }
      model->addConstr(expr == z[j][c1]);
    }
  }

}

void Model::defineConstraintFour() {

  for (int i = 0; i < nConnections; i++) {
    for (int c = 0; c < nChannels; c++) {

      GRBLinExpr expr;

      for (int u = 0; u < nConnections; u++) {
        if (u != i) {
          expr += interferenceMatrix[i][u] * z[u][c];
        }
      }

      model->addConstr(IC[i][c] == expr);
    }
  }
}

void Model::defineConstraintFive() {

  for (int i = 0; i < nConnections; i++) {
    for (int c = 0; c < nChannels; c++) {
      GRBLinExpr expr;

      expr = IC[i][c] - M_ij[i] * (1 - x[i][c]);

      model->addConstr(I[i] >= expr);
    }
  }

}

void Model::defineConstraintSix() {

  for (int i = 0; i < nConnections; i++) {

    for (int c = 0; c < nChannels; c++) {
      GRBLinExpr expr;

      expr = IC[i][c] + M_ij[i] * (1 - x[i][c]);

      model->addConstr(I[i] <= expr);
    }
  }
}

void Model::defineConstraintSeven() {
  for (int i = 0; i < nConnections; i++) {
    GRBLinExpr expr;

    for (int b = 0; b < 4; b++) {

      if (b == 0) {
        nDataRates = 9;
      } else {
        nDataRates = 10;
      }

      for (int s = 0; s < nDataRates; s++) {

        expr += ((connections[i].powerSR / SINR[s][b]) - noise) * y[i][b][s];
      }
    }

    model->addConstr(expr >= I[i]);
  }
}

void Model::defineConstraintEight() {
  for (int i = 0; i < nConnections; i++) {

    for (int u = 0; u < nConnections; u++) {
      for (int c = 0; c < nChannels; c++) {
        if (i != u) {
          model->addConstr(w[u][u] >= x[i][c] + z[u][c] - 1);
        }
      }
    }
  }
}

void Model::defineConstraintNine() {
  for (int i = 0; i < nConnections; i++) {
    for (int c = 0; c < nChannels; c++) {

      GRBLinExpr expr;

      for (int u = 0; u < nConnections; u++) {
        if (u != i) {
          expr += interferenceMatrix[i][u] * w[u][u];
        }
      }

      model->addConstr(I[i] == expr);
    }
  }
}

void Model::defineConstraints() {
  if (_type == nonlinear) {

  } else if (_type == linear_bigM) {
    defineConstraintOne();
    defineConstraintTwo();
    defineConstraintThree();
    defineConstraintFour();
    defineConstraintFive();
    defineConstraintSix();
    defineConstraintSeven();
  } else if (_type == linearV1) {
    defineConstraintOne();
    defineConstraintTwo();
    defineConstraintThree();
    defineConstraintSeven();
    defineConstraintEight();
    defineConstraintNine();
  }
}


void Model::createBigM() {

  for (int c = 0; c < nConnections; c++) {
    double value = computeInterference(c);
    M_ij.emplace_back(value);
  }
}

inline double Model::computeInterference(double c) {
  double ret = 0.0;

  for (int i = 0; i < nConnections; i++) {

    if (c != i) {
      ret += interferenceMatrix[c][i];
    }
  }

  return ret;
}

void Model::defineObjectiveFunction() {
  GRBLinExpr function;

//  for (int i = 0; i < nConnections; i++) {

  for (int j = 0; j < nConnections; j++) {

    for (int b = 0; b < 4; b++) {

      for (int s = 0; s < 10; s++) {

//          if (i == j) {
        function += dataRates[s][b] * y[j][b][s];
//          } else {
//            function += 0 * y[j][b][s];
//          }
      }
    }
  }
//  }


  try {
    model->setObjective(function, GRB_MAXIMIZE);
  } catch (GRBException e) {
    std::cout << e.getErrorCode() << std::endl;
    std::cout << e.getMessage() << std::endl;
    exit(-1);
  }
}

void Model::solve() {
  model->optimize();

  try {
  } catch (GRBException e) {
    std::cout << e.getMessage() << std::endl;
  }
}

int Model::getStatus() {
  return model->get(GRB_IntAttr_Status);
}

double Model::convertDBMToMW(double _value) {
  double result = 0.0, b;

  b = _value / 10.0;// dBm dividido por 10
  result = pow(10.0, b);//Converte de DBm para mW

  return result;
}

void
Model::convertTableToMW(const std::vector<std::vector<double> > &_SINR, std::vector<std::vector<double> > &_SINR_Mw,
                        int _lines,
                        int _rows) {
  double result, b;

  for (int i = 0; i < _SINR_Mw.size(); i++) {
    for (int j = 0; j < _SINR_Mw[i].size(); j++) {

      if (_SINR[i][j] != 0) {
        b = _SINR[i][j] / 10.0;// dBm divided by 10
        result = pow(10.0, b);//Convert DBM to mW

        _SINR_Mw[i][j] = result;
      } else {
        _SINR_Mw[i][j] = 0;
      }
    }

  }

}

void Model::printResults() {
  printf("Salvando no arquivo %s\n", outputFile.c_str());
  using namespace std;
  vector<std::string> rows;

  int opt = 0, gap = 1, obj = 2, objb = 3, runtime = 4, vars = 5;
  FILE *files[6];

  string arr[] = {"opt", "gap", "obj", "objb", "runtime", "vars"};

  for (int i = 0; i < 6; i++) {
    string path = outputFile + arr[i] + "_" + to_string(inst) + ".txt";
    files[i] = fopen(path.c_str(), "a");

    if (files[i] == nullptr) {
      puts("deu bosta");
      exit(EXIT_FAILURE);
    }
  }

  if (getStatus() == GRB_OPTIMAL) {
    fprintf(files[opt], "1\n");
  } else fprintf(files[opt], "0\n");

  try {
    fprintf(files[gap], "%lf\n", model->get(GRB_DoubleAttr_MIPGap));
    fprintf(files[obj], "%lf\n", model->get(GRB_DoubleAttr_ObjVal));
    fprintf(files[objb], "%lf\n", model->get(GRB_DoubleAttr_ObjBound));
    fprintf(files[runtime], "%lf\n", model->get(GRB_DoubleAttr_Runtime));
    printXVariables(&files[vars]);
    printYVariables(&files[vars]);
    printIVariables(&files[vars]);

    if (_type == linear_bigM) {
      printICVariables(&files[vars]);
      printZVariables(&files[vars]);
    } else if (_type == linearV1) {
      printWVariables(&files[vars]);
    }
  } catch (GRBException e) {
    std::cout << e.getMessage() << std::endl;
  }

  for (int i = 0; i < 6; i++)
    fclose(files[i]);
}

double Model::getObjVal() {
  return model->get(GRB_DoubleAttr_ObjVal);
}

double Model::getObjBound() {
  return model->get(GRB_DoubleAttr_ObjBound);
}

double Model::getMIPGap() {
  return model->get(GRB_DoubleAttr_MIPGap);
}

double Model::getRuntime() {
  return model->get(GRB_DoubleAttr_Runtime);
}

void Model::turnOffLogConsole(bool flag) {
  if (flag) {
    model->set(GRB_IntParam_OutputFlag, 0);
  } else {
    model->set(GRB_IntParam_OutputFlag, 1);
  }
}

void Model::setLogToMyDefaultFile() {
  model->set(GRB_StringParam_LogFile, outputFile + "gurobiOUT_" + std::to_string(inst) + ".txt");
}

void Model::printXVariables(FILE **out) {
  fprintf(*out, "==== X VARIABLES ====\n");
  for (int i = 0; i < nConnections; i++) {
    for (int c = 0; c < nChannels; c++) {
      if (x[i][c].get(GRB_DoubleAttr_X) > 0.0) {
        std::string outs_ =
                x[i][c].get(GRB_StringAttr_VarName) + ": " + std::to_string(x[i][c].get(GRB_DoubleAttr_X)) + "\n";
        fprintf(*out, outs_.c_str());
      }
    }
  }
}

void Model::printYVariables(FILE **out) {
  fprintf(*out, "==== Y VARIABLES ====\n");
  for (int i = 0; i < nConnections; i++) {
    for (int b = 0; b < 10; b++) {
      int dataRates = (b == 0) ? 9 : 10;

      for (int s = 0; s < dataRates; s++) {
        if (y[i][b][s].get(GRB_DoubleAttr_X) > 0.0) {
          std::string outs_ =
                  y[i][b][s].get(GRB_StringAttr_VarName) + ": " + std::to_string(y[i][b][s].get(GRB_DoubleAttr_X)) +
                  "\n";
          fprintf(*out, outs_.c_str());
        }
      }
    }
  }
}

void Model::printZVariables(FILE **out) {
  fprintf(*out, "==== Z VARIABLES ====\n");
  for (int i = 0; i < nConnections; i++) {
    for (int c = 0; c < nChannels; c++) {
      if (z[i][c].get(GRB_DoubleAttr_X) > 0.0) {
        std::string outs_ =
                z[i][c].get(GRB_StringAttr_VarName) + ": " + std::to_string(z[i][c].get(GRB_DoubleAttr_X)) + "\n";
        fprintf(*out, outs_.c_str());

        for (int k = 0; k < nChannels; k++) {
          if (overlap[c][k] && x[i][k].get(GRB_DoubleAttr_X) > 0.0) {
            std::string outs_2 = "  -----> " +
                    x[i][k].get(GRB_StringAttr_VarName) + ": " + std::to_string(x[i][k].get(GRB_DoubleAttr_X));

            fprintf(*out, outs_2.c_str());
          }
        }
      }
    }
  }
}

void Model::printICVariables(FILE **out) {

}

void Model::printIVariables(FILE **out) {

}

void Model::printWVariables(FILE **out) {

}
