//
// Created by José Joaquim on 2019-05-23.
//

#ifndef VRBSP_STRUCTURES_H
#define VRBSP_STRUCTURES_H

//TODO: Make a constructor that is padronized for each structure.

typedef struct Coordenate {
    double x, y;

    Coordenate(const Coordenate &aux) : x(aux.x), y(aux.y) {}

    Coordenate(double x, double y) : x(x), y(y) {}

    Coordenate() {}

}Coordenate;

struct Connection {

    Connection() {
      id = 0;
      bandwidth = 0;
      frequency = 0;
      idDataRate = 0;
      powerSR = 0.0;
      valureSINR = 0.0;
      distanceSenderReceptor = 0.0;
      totalInterference = 0.0;
      active = true;

    }

    int id;
    int bandwidth;
    int frequency;
    int idDataRate;
    double powerSR;//Valor da potência enviada do transmissor Si até o receptor Ri da conexão Li
    double valureSINR;
    double distanceSenderReceptor;
    double totalInterference;//Store the the interference of the all connection in the used channel
    bool active;
};

struct Interference {
    int idSender;
    int idReceptor;
    double distanceConnections;
    double valueInterference;
};

struct Channel {
    int bandwidth;
    int frequency;
    double throughput;
    std::vector<Connection> listConnections;
};

struct Spectrum {
    int maxSpectrum;
    int usedSpectrum;
    std::vector<Channel> listChannels;
};

#endif //VRBSP_STRUCTURES_H