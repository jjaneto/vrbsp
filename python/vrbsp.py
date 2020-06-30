#! /usr/bin/env python3
# TODO: check if the values of distanceMatrix,
#       interferenceMatrix, and M_ij are set right.

import gurobipy as gp
import math
from gurobipy import GRB

overlap = []
nChannels = 45
count_inst = 0


def converTableToMW(SINR, SINR_):
    for i in range(len(SINR_)):
        for j in range(len(SINR_[i])):
            if SINR[i][j] != 0.0:
                b = SINR[i][j] / 10.0
                result = math.pow(10.0, b)

                SINR_[i][j] = result
            else:
                SINR_[i][j] = 0.0


def distance(a, b, c, d):
    return math.hypot((a - c), (b - d))


def distanceAndInterference(
    senders,
    receivers,
    interferenceMatrix,
    distanceMatrix,
    powerSender,
    nConnections,
    alfa,
):
    for i in range(nConnections):
        distanceMatrix.append([])
        interferenceMatrix.append([])
        X_si = receivers[i][0]
        Y_si = receivers[i][1]

        for j in range(nConnections):
            X_rj = senders[j][0]
            Y_rj = senders[j][1]

            dist = distance(X_si, Y_si, X_rj, Y_rj)
            distanceMatrix[i].append(dist)

            if i == j:
                interferenceMatrix[i].append(0.0)
            else:
                value = powerSender / math.pow(dist, alfa) if dist != 0.0 else 1e9
                interferenceMatrix[i].append(value)


def cToB(c):
    if c < 25:
        return 0
    if c < 37:
        return 1
    if c < 43:
        return 2

    return 3


def computeInterference(c, nConnections):
    ret = 0.0
    for i in range(nConnections):
        if c != i:
            ret += interferenceMatrix[c][i]

    return ret


def createBigM(M_ij, nConnections):
    for c in range(nConnections):
        value = computeInterference(c, nConnections)
        M_ij.append(value)


def converDBMToMW(noise):
    b = noise / 10.0
    result = math.pow(10.0, b)
    return result


def loadData(
    path,
    receivers,
    senders,
    dataRates,
    SINR,
    spectrums,
    interferenceMatrix,
    distanceMatrix,
    M_ij,
):
    with open(path, "r") as f:
        line = f.readline()
        line = f.readline()
        aux = line.split()
        nConnections = int(aux[0])
        ttm = float(aux[1])
        alfa = float(aux[2])
        noise = float(aux[3])
        powerSender = float(aux[4])
        nSpectrums = float(aux[5])

        spectrums.append(int(aux[6]))
        spectrums.append(int(aux[7]))
        spectrums.append(int(aux[8]))

        # eu_li = str(nConnections) + " " + str(ttm) + " " + str(alfa) + " "
        # eu_li += str(noise) + " " + str(powerSender) + " " + str(nSpectrums) + " "
        # for i in range(3):
        #    eu_li += str(spectrums[i]) + " "
        #
        # print(eu_li)

        if noise != 0:
            noise = converDBMToMW(noise)

        f.readline()
        for i in range(10):
            line = f.readline()
            aux = line.split()

            dataRates.append([])
            for j in range(4):
                dataRates[i].append(float(aux[j]))

        f.readline()
        for i in range(10):
            line = f.readline()
            aux = line.split()

            SINR.append([])
            for j in range(4):
                SINR[i].append(float(aux[j]))

        converTableToMW(SINR, SINR)

        f.readline()
        for i in range(nConnections):
            line = f.readline()
            aux = line.split()
            receivers.append([float(aux[0]), float(aux[1])])

        del receivers[0]

        f.readline()
        for i in range(nConnections):
            line = f.readline()
            aux = line.split()
            senders.append([float(aux[0]), float(aux[1])])

        del senders[0]

        distanceAndInterference(
            senders,
            receivers,
            interferenceMatrix,
            distanceMatrix,
            powerSender,
            nConnections,
            alfa,
        )

        # for i in range(nConnections):
        #     for j in range(nConnections):
        #         print(interferenceMatrix[i][j], end=" ")
        #     print()

        createBigM(M_ij, nConnections)

    return noise, powerSender, alfa, nConnections


def defineVariables(model, model_type, nConnections, x, z, w, I, I_c):
    global nChannels

    for i in range(nConnections):
        for c in range(nChannels):
            for m in range(10):
                name = "x[" + str(i) + "][" + str(c) + "][" + str(m) + "]"
                x[i, c, m] = model.addVar(0.0, 1.0, 1.0, GRB.BINARY, name)

    for i in range(nConnections):
        for c in range(nChannels):
            name = "z[" + str(i) + "][" + str(c) + "]"
            z[i, c] = model.addVar(0.0, 1.0, 0.0, GRB.BINARY, name)

    for i in range(nConnections):
        name = "I[" + str(i) + "]"
        I[i] = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, name)

    if model_type == 0:
        for i in range(nConnections):
            for c in range(nChannels):
                name = "I_c[" + str(i) + "][" + str(c) + "]"
                I_c[i, c] = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, name)
    elif model_type == 1:
        for i in range(nConnections):
            for j in range(nConnections):
                name = "w[" + str(i) + "][" + str(c) + "]"
                w[i, c] = model.addVar(0.0, 1.0, 0.0, GRB.CONTINUOUS, name)


def defineConstraints(
    model, model_type, nConnections, SINR, powerSender, noise, x, z, w, I, I_c,
):
    global overlap, nChannels

    # Constraint 1
    for i in range(nConnections):
        expr = gp.LinExpr()
        for c in range(nChannels):
            nMCS = 10 if cToB(c) != 0 else 9
            for m in range(nMCS):
                expr = expr + x[i, c, m]

        model.addConstr(expr <= 1.0)

    # Constraint 2
    for i in range(nConnections):
        for c1 in range(nChannels):
            nMCS = 10 if cToB(c1) != 0 else 9
            exp = gp.LinExpr()
            for c2 in range(nChannels):
                for m in range(nMCS):
                    if overlap[c1][c2] == 1:
                        exp += x[i, c2, m]
            model.addConstr(exp == z[i, c1])

    # Constraint 3
    for i in range(nConnections):
        for c in range(nChannels):
            exp = gp.LinExpr()
            for u in range(nConnections):
                if i != u:
                    exp += interferenceMatrix[i][u] * z[u, c]
            model.addConstr(I_c[i, c] == exp)

    # Constraint 4
    for i in range(nConnections):
        for c in range(nChannels):
            nDataRates = 10 if cToB(c) != 0 else 9
            for m in range(nDataRates):
                lin_exp = gp.LinExpr()
                lin_exp = I_c[i, c] - M_ij[i] * (1 - x[i, c, m])
                model.addConstr(I[i] >= lin_exp)

    # Constraint 5
    for i in range(nConnections):
        for c in range(nChannels):
            nDataRates = 10 if cToB(c) != 0 else 9
            for m in range(nDataRates):
                lin_exp = gp.LinExpr()
                lin_exp = I_c[i, c] + M_ij[i] * (1 - x[i, c, m])
                model.addConstr(I[i] <= lin_exp)

    # Constraint 6
    for i in range(nConnections):
        expr = gp.LinExpr()
        for c in range(nChannels):
            nDataRates = 9 if cToB(c) == 0 else 10
            for m in range(nDataRates):
                value = powerSender / math.pow(distanceMatrix[i][i], alfa)
                value /= SINR[m][cToB(c)]
                value -= noise
                expr += value * x[i, c, m]

        model.addConstr(expr >= I[i])


def defineObjectiveFunction(model, model_type, nConnections, dataRates, x):
    global nChannels
    objFunction = gp.LinExpr()

    if model_type == 0:
        for i in range(nConnections):
            for c in range(nChannels):
                nMCS = 10 if cToB(c) != 0 else 9
                for m in range(nMCS):
                    objFunction += dataRates[m][cToB(c)] * x[i, c, m]

        model.setObjective(objFunction, GRB.MAXIMIZE)
    elif model_type == 1:
        print("TODO...")


def modelF1_v2(
    model_type,
    nConnections,
    interferenceMatrix,
    distanceMatrix,
    dataRates,
    SINR,
    powerSender,
    noise,
):
    global count_inst
    try:
        # Create a new model
        model = gp.Model("vrbsp")
        # Variaveis: x, z, I, I_c, w
        x, z, w = {}, {}, {}
        I, I_c = {}, {}
        defineVariables(model, model_type, nConnections, x, z, w, I, I_c)

        defineConstraints(
            model, model_type, nConnections, SINR, powerSender, noise, x, z, w, I, I_c,
        )
        defineObjectiveFunction(model, model_type, nConnections, dataRates, x)

        file_name = "out-formatted" + str(count_inst) + ".txt"
        model.write("./formulation.lp")
        model.setParam("LogFile", file_name)
        model.setParam(GRB.Param.LogToConsole, False)
        model.optimize()
        with open("objectives.txt", "a") as obj_file:
            obj_file.write(str(model.getAttr(GRB.Attr.ObjVal)) + "\n")
        model.write("./solution.sol")

        # conn, canal, bw, interference
        with open(file_name, "a") as f:
            for i in range(nConnections):
                for c in range(nChannels):
                    nMCS = 10 if cToB(c) != 0 else 9
                    for m in range(nMCS):
                        if x[i, c, m].getAttr("x") == 1.0:
                            # print(str(i) + " to na " + str(b) + " " + str(m))
                            f.write(
                                "%d %d %d %d %.12lf\n"
                                % (i, c, cToB(c), m, I[i].getAttr("x"))
                            )

    except gp.GurobiError as e:
        print("Error code " + str(e.errno) + ": " + str(e))

    except AttributeError:
        print("Encountered an attribute error")


def loadOverlap():
    with open("./overlap.txt", "r") as f:
        idx = 0
        for line in f:
            aux = line.split(",")
            arr_aux = []
            for j in range(len(aux)):
                arr_aux.append(int(aux[j]))

            overlap.append(arr_aux)
            idx += 1


if __name__ == "__main__":
    loadOverlap()
    for idx in range(12, 15):
        receivers, senders, dataRates = [[]], [[]], [[]]
        SINR, spectrums = [], []
        distanceMatrix, interferenceMatrix = [[]], [[]]
        M_ij = []

        # inst = idx + 1
        inst = idx + 1
        count_inst = inst
        path = "./D10000x10000/U_256/U_256_" + str(inst) + ".txt"
        noise, powerSender, alfa, nConnections = loadData(
            path,
            receivers,
            senders,
            dataRates,
            SINR,
            spectrums,
            interferenceMatrix,
            distanceMatrix,
            M_ij,
        )

        modelF1_v2(
            int(0),
            nConnections,
            interferenceMatrix,
            distanceMatrix,
            dataRates,
            SINR,
            powerSender,
            noise,
        )
