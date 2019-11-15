import pandas as pd
import os


if __name__ == '__main__':
    all_data_file = os.listdir('data/')
    for file in all_data_file:
        df = pd.read_csv('data/' + file, encoding='big5')
        name = []
        for x in list(df):
            for tmp in [' ', '\t', '\n', '\r']:
                x = x.replace(tmp, '')
            name.append(x)
        df.columns = name
        df = df.loc[:, ['商品代號', '履約價格', '到期月份(週別)', '買賣權別', '成交時間', '成交價格', '成交數量(BorS)']]
        df['商品代號'] = [str(x).replace(' ', '') for x in df['商品代號']]
        df = df[df['商品代號'] == 'TXO']
        all_contract = [str(x).replace(' ', '') for x in df['到期月份(週別)'].unique()]
        df['到期月份(週別)'] = [str(x).replace(' ', '') for x in df['到期月份(週別)']]
        target_contract = ''
        all_contract.sort()
        print(all_contract)
        for x in all_contract:
            if 'W' in x:
                target_contract = x
                break
        df = df[df['到期月份(週別)'] == target_contract]
        df['買賣權別'] = [x.replace(' ', '') for x in df['買賣權別']]
        if len(df) > 0:
            df.to_csv('TXO/' + file[-14:-4] + '.csv', index=False, encoding='big5')
