import itertools
from itertools import product
import math


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


def checkArbitrage(test_dataC, test_dataP, testCX, testPX):
    '''
    商品代號  履約價格  到期月份(週別) 買賣權別   成交時間   成交價格  成交數量(BorS)
    0   TXO  8400  201701W1    C  84611  735.0          20
    1   TXO  8400  201701W1    C  84611  735.0          20
    2   TXO  9200  201701W1    C  84611   79.0          17
    3   TXO  9200  201701W1    C  84611   79.0           1
    4   TXO  9200  201701W1    C  84611   79.0           1
    5   TXO  9200  201701W1    C  84611   79.0          10
    6   TXO  9200  201701W1    C  84611   79.0           5
    7   TXO  9200  201701W1    P  84611   11.5           1
    8   TXO  9200  201701W1    P  84611   11.5           1
    9   TXO  9200  201701W1    P  84611   11.5           1
    10  TXO  9200  201701W1    P  84611   11.5           1
    '''

    '''
    (x3-x2)Cx1 + (x2-x1)Cx3 - (x3-x1)Cx2 >= Cs
    (x3-x2)Px1 + (x2-x1)Px3 - (x3-x1)Px2 >= Cs
    '''
    C_count = decide(test_dataC, testCX)
    P_count = decide(test_dataP, testPX)

    return C_count, P_count


def serial(test_dataC, test_dataP, testCX, testPX):
    C_count, P_count = checkArbitrage(test_dataC, test_dataP, testCX, testPX)
    print("C count is {}".format(C_count))
    print("P count is {}".format(P_count))
