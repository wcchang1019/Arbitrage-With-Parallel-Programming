import pandas as pd
import os
from serial import serial, decide
from parallel_cuda import parallelCuda
import multiprocessing


def readCsv(filename):
    data = pd.read_csv('TXO/' + filename, encoding='big5')
    data['履約價格'] = data['履約價格'].astype(int)
    data['成交數量(BorS)'] = data['成交數量(BorS)'].astype(int)
    unique_time = pd.unique(data['成交時間'])
    unique_time.sort()
    return data, unique_time


def selectData(data, unique_time):
    test_data = data[data['成交時間'] == unique_time].reset_index(drop=True)
    test_dataC = test_data[test_data['買賣權別'] == 'C'].reset_index(drop=True)
    test_dataP = test_data[test_data['買賣權別'] == 'P'].reset_index(drop=True)
    testCX = pd.unique(test_dataC['履約價格'])
    testCX.sort()
    testPX = pd.unique(test_dataP['履約價格'])
    testPX.sort()
    return unique_time, test_dataC[['履約價格', '成交數量(BorS)', '成交價格']], test_dataP[['履約價格', '成交數量(BorS)', '成交價格']], testCX, testPX


if __name__ == '__main__':
    all_data_file = os.listdir('TXO/')
    all_data_file.sort()
    p = multiprocessing.Pool(2)
    count = 0
    total_C = 0
    total_P = 0
    for file in all_data_file[:1]:
        print(file)
        data, unique_time = readCsv(file)
        for t in unique_time:
            unique_time, test_dataC, test_dataP, testCX, testPX = selectData(data, t)
            C_count, P_count = serial(test_dataC, test_dataP, testCX, testPX)
            total_C += C_count
            total_P += P_count

            
            if C_count != 0 or P_count != 0:
                print("count = {}".format(count))
                print("serial c count = {}".format(C_count))
                print("serial p count = {}".format(P_count))
            
            pthread_C_count = p.apply_async(decide, args = (test_dataC, testCX))
            pthread_P_count = p.apply_async(decide, args = (test_dataP, testPX))
            total_C += pthread_C_count.get()
            total_P += pthread_P_count.get()

            if pthread_C_count.get() != 0 or pthread_P_count.get() != 0:
                print("count = {}".format(count))
                print("pthread_C_count = {}".format(pthread_C_count.get()))
                print("pthread_P_count = {}".format(pthread_P_count.get()))

            # cuda_C_count, cuda_P_count = parallelCuda(test_dataC, test_dataP, testCX, testPX)
            # total_C += cuda_C_count
            # total_P += cuda_P_count

            # if count == 3000:
            #     break
            count += 1
        break
    p.close()
    p.join()

    print("pthread_C_count count is {}".format(total_C))
    print("pthread_P_count count is {}".format(total_P))