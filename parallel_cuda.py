import itertools
from itertools import product
import math
from numba import cuda

@cuda.jit
def decide(test_dataC, testCX):
    count = 0

    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x

    CX3 = test_dataC[test_dataC['履約價格'] == testCX[idx][2]].reset_index(drop=True)
    CX2 = test_dataC[test_dataC['履約價格'] == testCX[idx][1]].reset_index(drop=True)
    CX1 = test_dataC[test_dataC['履約價格'] == testCX[idx][0]].reset_index(drop=True)

    price_combination = []
    price_combination.append([_ for _ in range(len(CX1))])
    price_combination.append([_ for _ in range(len(CX2))])
    price_combination.append([_ for _ in range(len(CX3))])
    for combinations_of_p in list(product(*price_combination)) :
        volumeCX1 = testCX[idx][2] - testCX[idx][1]
        volumeCX2 = testCX[idx][2] - testCX[idx][0]
        volumeCX3 = testCX[idx][1] - testCX[idx][0]
        gcd = math.gcd( math.gcd(volumeCX1, volumeCX2), volumeCX3)
        volumeCX1 /= gcd
        volumeCX2 /= gcd
        volumeCX3 /= gcd
        if volumeCX1 > CX1['成交數量(BorS)'][combinations_of_p[0]] or volumeCX2 > CX2['成交數量(BorS)'][combinations_of_p[1]] or volumeCX3 > CX3['成交數量(BorS)'][combinations_of_p[2]] :
            continue
        if  ((volumeCX1) * CX1['成交價格'][combinations_of_p[0]] + \
            (volumeCX3) * CX3['成交價格'][combinations_of_p[2]] - \
            (volumeCX2) * CX2['成交價格'][combinations_of_p[1]]) >= 20*(volumeCX1+volumeCX2+volumeCX3) :
            count += 1
    return count

def parallelCuda(test_dataC, test_dataP, testCX, testPX):
    n = len(testCX) * 3
    threads_per_block = n 
    blocks_per_grid = math.ceil(n / threads_per_block)

    dataCX = []
    dataPX = []
    for combinations in itertools.combinations(testCX,3) :
        dataCX.append([combinations])
    for combinations in itertools.combinations(testPX,3) :
        dataPX.append([combinations])
    C_count = decide[blocks_per_grid, threads_per_block](test_dataC,dataCX)
    P_count = decide[blocks_per_grid, threads_per_block](test_dataP,dataPX)

    return C_count, P_count
