import itertools
from itertools import product
import math
import numpy as np


def decide(test_dataC, testCX):
    count = 0
    for combinations_x in itertools.combinations(testCX,3) :
        CX3 = test_dataC[test_dataC['履約價格'] == combinations_x[2]].reset_index(drop=True)
        CX2 = test_dataC[test_dataC['履約價格'] == combinations_x[1]].reset_index(drop=True)
        CX1 = test_dataC[test_dataC['履約價格'] == combinations_x[0]].reset_index(drop=True)

        price_combination = []
        price_combination.append([_ for _ in range(len(CX1))])
        price_combination.append([_ for _ in range(len(CX2))])
        price_combination.append([_ for _ in range(len(CX3))])
        for combinations_of_p in list(product(*price_combination)) :
            volumeCX1 = combinations_x[2] - combinations_x[1]
            volumeCX2 = combinations_x[2] - combinations_x[0]
            volumeCX3 = combinations_x[1] - combinations_x[0]
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


def serial(test_dataC, test_dataP, testCX, testPX):
    C_count = decide(test_dataC, testCX)
    P_count = decide(test_dataP, testPX)
    return C_count, P_count