import itertools
from itertools import product
import math
from numba import cuda
import numpy as np

@cuda.jit
def compute_price(CX1, CX2, CX3, price_combination):
    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x
    for i in range (len(CX1)):
        price_combination = np.append(price_combination, i)
    for i in range (len(CX2)):
        price_combination = np.append(price_combination, i)
    for i in range (len(CX3)):
        price_combination = np.append(price_combination, i)

@cuda.jit
def compute_CX1(test_dataC, testCX, CX1):
    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x
    for index in range (len(test_dataC)):
        if test_dataC[index] == testCX[idx][2]:
            np.append(CX1,test_dataC[index], axis=0)

def compute_CX1_serial(test_dataC, testCX, CX1):
    # idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x
    for index in range (len(test_dataC)):
        if test_dataC[index] == testCX[idx][2]:
            np.append(CX1,test_dataC[index], axis=0)

@cuda.jit
def compute_CX2(test_dataC, testCX, CX2):
    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x
    for index in range (len(test_dataC)):
        print(testCX[idx][2])
        if test_dataC[index][0] == testCX[idx][2]:
            np.append(CX2,test_dataC[index], axis=0)

@cuda.jit
def compute_CX3(test_dataC, testCX, CX3):
    idx = cuda.threadIdx.x + cuda.blockDim.x * cuda.blockIdx.x
    for index in range (len(test_dataC)):
        print(testCX[idx][2])
        if test_dataC[index][0] == testCX[idx][2]:
            np.append(CX3,test_dataC[index], axis=0)

def decide_not_cuda(testCX, CX1, CX2, CX3, price_combination, idx):
    count = 0
    # print(np.meshgrid(price_combination))
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
        dataCX.append(combinations)
    for combinations in itertools.combinations(testPX, 3) :
        dataPX.append(combinations)
    dataCX = np.array(dataCX)
    dataPX = np.array(dataPX)

    test_dataC_array = test_dataC.values.copy()
    test_dataC_array0 = test_dataC_array[:,0].astype(int)

    test_dataP_array = test_dataP.values.copy()
    test_dataP_array[:,0] = test_dataP_array[:,0].astype(int)

    CX3 = cuda.to_device(np.array([]))
    CX2 = cuda.to_device(np.array([]))
    CX1 = cuda.to_device(np.array([]))
    price_combination = cuda.to_device(np.array([]))

    print(test_dataC_array.shape)
    print(test_dataC_array0.shape)
    print(type(test_dataC_array0[0]))

    compute_CX1[blocks_per_grid, threads_per_block](test_dataC_array0, dataCX, CX1)
    # compute_CX2[blocks_per_grid, threads_per_block](test_dataC_array, dataCX, CX2)
    # compute_CX3[blocks_per_grid, threads_per_block](test_dataC_array, dataCX, CX3)

    print(CX1.copy_to_host())
    # print(CX2.copy_to_host())
    # print(CX3.copy_to_host())


    ##############################################################################

    # price_combination = compute_price[blocks_per_grid, threads_per_block](CX1, CX2, CX3, price_combination)
    # # C_count = decide_not_cuda(test_dataC_array, CX1, CX2, CX3, price_combination)

    # CX1 = compute_CX[blocks_per_grid, threads_per_block](test_dataP_array, dataPX, CX1)
    # CX2 = compute_CX[blocks_per_grid, threads_per_block](test_dataP_array, dataPX, CX2)
    # CX3 = compute_CX[blocks_per_grid, threads_per_block](test_dataP_array, dataPX, CX3)
    # price_combination = compute_price[blocks_per_grid, threads_per_block](CX1, CX2, CX3, price_combination)
    # # P_count = decide_not_cuda(test_dataP_array, CX1, CX2, CX3, price_combination)

    C_count = 0
    P_count = 0


    # C_count = decide[blocks_per_grid, threads_per_block](test_dataC_array, dataCX, CX3, CX2, CX1, price_combination)
    # P_count = decide[blocks_per_grid, threads_per_block](test_dataP_array, dataPX, CX3, CX2, CX1, price_combination)

    return C_count, P_count