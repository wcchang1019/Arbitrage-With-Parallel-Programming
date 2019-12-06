import pandas as pd
import os
from serial import serial


def readCsv(filename):
    data = pd.read_csv('TXO/' + filename, encoding='big5')
    data['履約價格'] = data['履約價格'].astype(int)
    data['成交數量(BorS)'] = data['成交數量(BorS)'].astype(int)
    unique_time = pd.unique(data['成交時間'])
    unique_time.sort()
    return data, unique_time


def selectData(data, unique_time):
    test_data = data[data['成交時間'] == unique_time].reset_index(drop=True)
    test_dataC = test_data[test_data['買賣權別'] == 'C']
    test_dataP = test_data[test_data['買賣權別'] == 'P']
    testCX = pd.unique(test_dataC['履約價格'])
    testCX.sort()
    testPX = pd.unique(test_dataP['履約價格'])
    testPX.sort()
    return unique_time, test_dataC, test_dataP, testCX, testPX


if __name__ == '__main__':
    all_data_file = os.listdir('TXO/')
    all_data_file.sort()
    for file in all_data_file[:1]:
        data, unique_time = readCsv(file)
        for t in unique_time:
            unique_time, test_dataC, test_dataP, testCX, testPX = selectData(data, t)
            serial(test_dataC, test_dataP, testCX, testPX)
