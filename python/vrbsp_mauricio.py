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


def computeInterference(c, nConnections, interferenceMatrix):
    ret = 0.0
    for i in range(nConnections):
        if c != i:
            ret += interferenceMatrix[c][i]

    return ret


def createBigM(M_ij, nConnections, interferenceMatrix):
    for c in range(nConnections):
        value = computeInterference(c, nConnections, interferenceMatrix)
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

        createBigM(M_ij, nConnections, interferenceMatrix)

    #         for i in range(10):
    #             for j in range(4):
    #                 print("%.12f " % (SINR[i][j]), end="")
    #             print()

    return noise, powerSender, alfa, nConnections


def defineVariables(model, model_type, nConnections, x, y, z, w, I, I_c):
    global nChannels

    for i in range(nConnections):
        for c in range(nChannels):
            name = "x[" + str(i) + "][" + str(c) + "]"
            x[i, c] = model.addVar(0.0, 1.0, 0.0, GRB.BINARY, name)

    for i in range(nConnections):
        for b in range(4):
            MCS = 10 if b != 0 else 9
            for m in range(MCS):
                name = "y[" + str(i) + "][" + str(b) + "][" + str(m) + "]"
                y[i, b, m] = model.addVar(0.0, 1.0, 1.0, GRB.BINARY, name)

    for i in range(nConnections):
        for c in range(nChannels):
            name = "z[" + str(i) + "][" + str(c) + "]"
            z[i, c] = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, name)

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
    model, model_type, nConnections, SINR, powerSender, noise, x, y, z, w, I, I_c,
):
    global overlap, nChannels
    C_b = [
        [
            0,
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12,
            13,
            14,
            15,
            16,
            17,
            18,
            19,
            20,
            21,
            22,
            23,
            24,
        ],
        [25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36],
        [37, 38, 39, 40, 41, 42],
        [43, 44],
    ]

    # Constraint 1
    for i in range(nConnections):
        expr = gp.LinExpr()
        for c in range(nChannels):
            expr += x[i, c]

        model.addConstr(expr <= 1.0)  # TODO: verificar isso aqui

    # Constraint 2
    for b in range(4):
        for i in range(nConnections):
            expr1, expr2 = gp.LinExpr(), gp.LinExpr()
            nMCS = 10 if b != 0 else 9
            for m in range(nMCS):
                expr1 += y[i, b, m]

            for ch in C_b[b]:
                expr2 += x[i, ch]

            model.addConstr(expr1 <= expr2)

    # Constraint 3
    for i in range(nConnections):
        for c1 in range(nChannels):
            exp = gp.LinExpr()
            for c2 in range(nChannels):
                if overlap[c1][c2] == 1:
                    exp += x[i, c2]

            model.addConstr(exp == z[i, c1])

    # Constraint 4
    for i in range(nConnections):
        for c in range(nChannels):
            exp = gp.LinExpr()
            for u in range(nConnections):
                if i != u:
                    exp += interferenceMatrix[i][u] * z[u, c]
            model.addConstr(I_c[i, c] == exp)

    # Constraint 5
    for i in range(nConnections):
        for c in range(nChannels):
            lin_exp = gp.LinExpr()
            lin_exp = I_c[i, c] - M_ij[i] * (1 - x[i, c])
            model.addConstr(I[i] >= lin_exp)

    # Constraint 6
    for i in range(nConnections):
        for c in range(nChannels):
            lin_exp = gp.LinExpr()
            lin_exp = I_c[i, c] + M_ij[i] * (1 - x[i, c])
            model.addConstr(I[i] <= lin_exp)

    # Constraint 7s
    for i in range(nConnections):
        expr = gp.LinExpr()
        for b in range(4):
            nDataRates = 9 if b == 0 else 10
            for m in range(nDataRates):
                value = powerSender / math.pow(distanceMatrix[i][i], alfa)
                value /= SINR[m][b]
                value -= noise
                expr += value * y[i, b, m]

        model.addConstr(expr >= I[i])


def defineObjectiveFunction(model, model_type, nConnections, dataRates, y):
    global nChannels
    objFunction = gp.LinExpr()

    if model_type == 0:
        for i in range(nConnections):
            for b in range(4):
                nMCS = 10 if b != 0 else 9
                for m in range(nMCS):
                    objFunction += dataRates[m][b] * y[i, b, m]

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
    global nChannels, count_inst
    try:
        # Create a new model
        model = gp.Model("vrbsp")
        # Variaveis: x, z, I, I_c, w
        x, y, z, w = {}, {}, {}, {}
        I, I_c = {}, {}
        defineVariables(model, model_type, nConnections, x, y, z, w, I, I_c)

        defineConstraints(
            model,
            model_type,
            nConnections,
            SINR,
            powerSender,
            noise,
            x,
            y,
            z,
            w,
            I,
            I_c,
        )
        defineObjectiveFunction(model, model_type, nConnections, dataRates, y)

        file_name = "out-formatted" + str(count_inst) + ".txt"
        model.write("./formulation" + str(count_inst) + ".lp")
        model.setParam("LogFile", file_name)
        model.setParam(GRB.Param.LogToConsole, False)
        model.setParam("TimeLimit", 3600)
        model.optimize()
        with open("result_information.txt", "a") as output_re:
            output_re.write(str(model.getAttr("ObjVal")) + " ")
            output_re.write(str(model.getAttr("ObjBoundC")) + " ")
            output_re.write(str(model.getAttr("MIPGap")) + " ")
            output_re.write(str(model.getAttr("NumVars")) + " ")
            output_re.write(str(model.getAttr("NumConstrs")) + " ")
            output_re.write(str(model.getAttr("IterCount")) + " ")
            output_re.write(str(model.getAttr("BarIterCount")) + " ")
            output_re.write(str(model.getAttr("NodeCount")) + " ")
            output_re.write(str(model.getAttr("Runtime")))
            output_re.write("\n")

        with open("objectives.txt", "a") as obj_file:
            obj_file.write(str(model.getAttr(GRB.Attr.ObjVal)) + "\n")
        model.write("./solution" + str(count_inst) + ".sol")

        # conn, canal, bw, interference
        with open(file_name, "a") as f:
            f.write(str(model.getAttr(GRB.Attr.ObjVal)) + "\n")
            for i in range(nConnections):
                for b in range(4):
                    nMCS = 10 if b != 0 else 9
                    for m in range(nMCS):
                        if y[i, b, m].getAttr("x") == 1.0:
                            # print(str(i) + " to na " + str(b) + " " + str(m))
                            normal = False
                            for c in range(nChannels):
                                if x[i, c].getAttr("x") == 1.0:
                                    normal = True
                                    f.write(
                                        "%d %d %d %d %.12f\n"
                                        % (i, c, b, m, I[i].getAttr("x"))
                                    )
                                    # print(i, c, b, m, I[i].getAttr("x"))

                            if not normal:
                                print("PROBLEMA")
                                exit()

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
    for idx in range(30):
        receivers, senders, dataRates = [[]], [[]], [[]]
        SINR, spectrums = [], []
        distanceMatrix, interferenceMatrix = [[]], [[]]
        M_ij = []

        inst = idx + 1
        print(inst)
        count_inst = inst
        path = "./D250x250/U_8/U_8_" + str(inst) + ".txt"
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
