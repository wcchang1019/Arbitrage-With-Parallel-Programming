import itertools
from collections import Counter
from itertools import product
import pandas as pd
import math

def readCsv(filename):
    data = pd.read_csv(filename, encoding='big5')
    data['履約價格'] = data['履約價格'].astype(int)
    data['成交數量(BorS)'] = data['成交數量(BorS)'].astype(int)

    unique_time = pd.unique(data['成交時間'])
    unique_time.sort()

    return data, unique_time

def selectData(data, unique_time):
    test_data = data[data['成交時間'] == unique_time[0]].reset_index(drop=True)
    test_dataC = test_data[test_data['買賣權別'] == 'C'] 
    test_dataP = test_data[test_data['買賣權別'] == 'P']

    '''
    [8400 9200 9250 9300 9350 9400]
    [9200 9250 9300]
    '''
    testCX = pd.unique(test_dataC['履約價格'])
    testCX.sort()
    testPX = pd.unique(test_dataP['履約價格'])
    testPX.sort()

    return unique_time, test_dataC, test_dataP, testCX, testPX

def decideC(test_dataC, testCX):
    C_count = 0
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
                C_count += 1   

    return C_count

def decideP(test_dataP, testPX):
    P_count = 0  
    for combinations_x in itertools.combinations(testPX,3) :
        PX3 = test_dataP[test_dataP['履約價格'] == combinations_x[2]].reset_index(drop=True)
        PX2 = test_dataP[test_dataP['履約價格'] == combinations_x[1]].reset_index(drop=True)
        PX1 = test_dataP[test_dataP['履約價格'] == combinations_x[0]].reset_index(drop=True)

        price_combination = []
        price_combination.append([_ for _ in range(len(PX1))])
        price_combination.append([_ for _ in range(len(PX2))])
        price_combination.append([_ for _ in range(len(PX3))])
        for combinations_of_p in list(product(*price_combination)) :
            volumePX1 = combinations_x[2] - combinations_x[1]
            volumePX2 = combinations_x[2] - combinations_x[0]
            volumePX3 = combinations_x[1] - combinations_x[0]
            gcd = math.gcd( math.gcd(volumePX1, volumePX2), volumePX3)
            volumePX1 /= gcd
            volumePX2 /= gcd
            volumePX3 /= gcd
            if volumePX1 > PX1['成交數量(BorS)'][combinations_of_p[0]] or volumePX2 > PX2['成交數量(BorS)'][combinations_of_p[1]] or volumePX3 > PX3['成交數量(BorS)'][combinations_of_p[2]] :
                continue
            if  ((volumePX1) * PX1['成交價格'][combinations_of_p[0]] + \
                (volumePX3) * PX3['成交價格'][combinations_of_p[2]] - \
                (volumePX2) * PX2['成交價格'][combinations_of_p[1]]) >= 20*(volumePX1+volumePX2+volumePX3) :
                P_count += 1

    return P_count

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
    C_count = decideC(test_dataC, testCX)
    P_count = decideP(test_dataP, testPX)

    return C_count, P_count

def serial():
    filename = 'TXO/2016_01_04.csv'
    data, unique_time = readCsv(filename)
    unique_time, test_dataC, test_dataP, testCX, testPX = selectData(data, unique_time)
    C_count, P_count = checkArbitrage(test_dataC, test_dataP, testCX, testPX)

    print("C count is {}".format(C_count))
    print("P count is {}".format(P_count))

if __name__ == '__main__':
    serial()