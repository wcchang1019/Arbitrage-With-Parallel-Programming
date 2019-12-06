import itertools
from itertools import product
import math
from numba import cuda
import numpy as np

@cuda.jit
def decide(test_dataC, testCX):
    count = 0

    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x

    #https://github.com/numba/numba/issues/4150
    CX3 = test_dataC[test_dataC[:,0] == testCX[idx][2]]
    CX2 = test_dataC[test_dataC[:,0] == testCX[idx][1]]
    CX1 = test_dataC[test_dataC[:,0] == testCX[idx][0]]

    price_combination = []
    price_combination.append([_ for _ in range(len(CX1))])
    price_combination.append([_ for _ in range(len(CX2))])
    price_combination.append([_ for _ in range(len(CX3))])
    price_combination = np.array(np.meshgrid(price_combination).T.reshape(-1,3))
    for combinations_of_p in price_combination :
        volumeCX1 = testCX[idx][2] - testCX[idx][1]
        volumeCX2 = testCX[idx][2] - testCX[idx][0]
        volumeCX3 = testCX[idx][1] - testCX[idx][0]
        gcd = math.gcd( math.gcd(volumeCX1, volumeCX2), volumeCX3)
        volumeCX1 /= gcd
        volumeCX2 /= gcd
        volumeCX3 /= gcd
        if volumeCX1 > CX1[1][combinations_of_p[0]] or volumeCX2 > CX2[1][combinations_of_p[1]] or volumeCX3 > CX3[1][combinations_of_p[2]] :
            continue
        if  ((volumeCX1) * CX1[2][combinations_of_p[0]] + \
            (volumeCX3) * CX3[2][combinations_of_p[2]] - \
            (volumeCX2) * CX2[2][combinations_of_p[1]]) >= 20*(volumeCX1+volumeCX2+volumeCX3) :
            count += 1
    return count

def parallelCuda(test_dataC, test_dataP, testCX, testPX):
    n = len(testCX) * 3
    threads_per_block = n 
    blocks_per_grid = math.ceil(n / threads_per_block)

    dataCX = []
    dataPX = []
    for combinations in itertools.combinations(testCX, 3) :
        dataCX.append([combinations])
    for combinations in itertools.combinations(testPX, 3) :
        dataPX.append([combinations])
    dataCX = np.array(dataCX)
    dataPX = np.array(dataPX)

    # dtype = [('name', 'int'), ('age', 'float'), ('weight', 'float')]
    # r = np.rec.fromrecords(test_dataC.values.copy(), names = 'name, age, weight')
    # test_dataC_array = r.astype(dtype)

    # test_dataC_array = np.array(test_dataC.values.copy(), dtype=dtype)
    test_dataC_array = test_dataC.values.copy()
    test_dataC_array[:,0] = test_dataC_array[:,0].astype(int)
    test_dataP_array = test_dataP.values.copy()
    test_dataP_array[:,0] = test_dataP_array[:,0].astype(int)

    print(test_dataC_array.dtype)
    print(testCX.dtype)

    C_count = decide[blocks_per_grid, threads_per_block](test_dataC_array, dataCX)
    P_count = decide[blocks_per_grid, threads_per_block](test_dataP_array, dataPX)

    return C_count, P_count













