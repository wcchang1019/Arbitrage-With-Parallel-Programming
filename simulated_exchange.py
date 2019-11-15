import pandas as pd
import os

if __name__ == '__main__':
    all_data_file = os.listdir('TXO/')
    all_data_file.sort()
    for file in all_data_file[:1]:
        df = pd.read_csv('TXO/' + file, encoding='big5')
        df['成交時間'] = df['成交時間'].astype(int)
        simulated_time = 84500
        while simulated_time <= 134500:
            now_data = df[df['成交時間'] == simulated_time]
            print(now_data)
            simulated_time += 1
            if simulated_time % 100 == 60:
                simulated_time += 40
            if simulated_time % 10000 == 6000:
                simulated_time += 4000
