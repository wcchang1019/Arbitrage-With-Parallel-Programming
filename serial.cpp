#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;


void ReadCsv(string fileName, vector<vector<string>>& allData, vector<int>& timeIdx)
{
    //readfile
    fstream file;
    file.open(fileName);
    string line;
    string timeStart = "84500";
    while (getline(file, line, '\n'))
    {
        istringstream templine(line); 
        string data;
        vector<string> row;
        int checkTime = 0;
        while (getline(templine, data, ',')) 
        {
            //cout << data.c_str() << endl;
            if(checkTime == 2 && data != timeStart)
            {
                timeIdx.push_back(allData.size()-1);
                timeStart = data;
            }
            row.push_back(data.c_str()); 
            checkTime++;
        }
        allData.push_back(row);
    }
    file.close();
}


void GetUniqueExercisePrice(vector<vector<string>> nowCallPutData, string cpType, vector<string>& uniqueExercisePrice, vector<vector<string>>& nowData)
{
    for(int i=0; i<nowCallPutData.size(); i++)
    {
        if(nowCallPutData[i][1] == cpType)
        {
            nowData.push_back(nowCallPutData[i]);
            if(find(uniqueExercisePrice.begin(), uniqueExercisePrice.end(), nowCallPutData[i][0]) == uniqueExercisePrice.end())
            {
                uniqueExercisePrice.push_back(nowCallPutData[i][0]);
            }
        }
    }
}


vector<vector<int>> Combination(int N, int K)
{
    vector<vector<int>> ans;
    string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's
    do {
        vector<int> row;
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) row.push_back(i);
        }
        ans.push_back(row);
    } while (prev_permutation(bitmask.begin(), bitmask.end()));
    return ans;
}


int ComputeArbitrage(vector<string> uniqueExercisePrice, vector<vector<string>> nowData)
{
    int ans = 0;
    if(uniqueExercisePrice.size() >= 3)
    {
        vector<vector<int>> allCombination;
        allCombination = Combination(uniqueExercisePrice.size(), 3);
        for(int x=0; x<allCombination.size(); x++)
        {
            vector<int> exercisePrice;
            vector<int> dataIdx1, dataIdx2, dataIdx3;
            exercisePrice.push_back(stoi(uniqueExercisePrice[allCombination[x][0]]));
            exercisePrice.push_back(stoi(uniqueExercisePrice[allCombination[x][1]]));
            exercisePrice.push_back(stoi(uniqueExercisePrice[allCombination[x][2]]));
            sort(exercisePrice.begin(), exercisePrice.end());
            /*for(int cnt=0; cnt<exercisePrice.size(); cnt++)
            {
                cout << x << ":" << exercisePrice[cnt] << "\t";
            }
            cout << endl;*/
            for(int i=0; i<nowData.size(); i++)
            {
                if(stoi(nowData[i][0]) == exercisePrice[0]) dataIdx1.push_back(i);
                if(stoi(nowData[i][0]) == exercisePrice[1]) dataIdx2.push_back(i);
                if(stoi(nowData[i][0]) == exercisePrice[2]) dataIdx3.push_back(i);
            }
            /*for(int i=0; i<dataIdx1.size(); i++)
            {
                cout << nowData[dataIdx1[i]][0] << " " << nowData[dataIdx1[i]][1] << " " << nowData[dataIdx1[i]][2] << " " << nowData[dataIdx1[i]][3] << " " << nowData[dataIdx1[i]][4] << endl;
            }*/
            //cout << (dataIdx1.size()) << "," << (dataIdx2.size()) << "," << (dataIdx3.size()) << endl;
            for(int a=0; a<dataIdx1.size(); a++)
            {
                for(int b=0; b<dataIdx2.size(); b++)
                {
                    for(int c=0; c<dataIdx3.size(); c++)
                    {
                        int tmp1, tmp2, tmp3, gcd;
                        tmp1 = exercisePrice[2] - exercisePrice[1];
                        tmp2 = exercisePrice[1] - exercisePrice[0];
                        tmp3 = exercisePrice[2] - exercisePrice[0];
                        gcd = __gcd(__gcd(tmp1, tmp2), tmp3);
                        tmp1 /= gcd;
                        tmp2 /= gcd;
                        tmp3 /= gcd;
                        //cout << tmp1 << "," << tmp2 << "," << tmp3 << endl;
                        if(tmp1 > stoi(nowData[dataIdx1[a]][4]) || tmp2 > stoi(nowData[dataIdx3[c]][4]) || tmp3 > stoi(nowData[dataIdx2[b]][4]))
                        {
                            //cout << "HAHA" << endl;
                            continue;
                        }
                        //cout << tmp1*stof(nowData[dataIdx2[b]][3]) << endl;
                        if(tmp1*stof(nowData[dataIdx1[a]][3])+tmp2*stof(nowData[dataIdx3[c]][3])-tmp3*stof(nowData[dataIdx2[b]][3]) >= 20*(tmp1+tmp2+tmp3))
                        {
                            //cout << begin << ":";
                            //cout << exercisePrice[2] << "-" << exercisePrice[1] << " " << stof(nowData[dataIdx1[a]][3]) << " " << exercisePrice[1] << "-" << exercisePrice[0] << " " << stof(nowData[dataIdx3[c]][3]) << " " << exercisePrice[2] << "-" << exercisePrice[0] << " " << stof(nowData[dataIdx1[a]][3]) << endl;
                            ans++;
                        }
                    }
                }
            }
        }
    }
    else
    {
        return 0;
    }
    return ans;
}

int main()
{
    //col1     col2        col3       col4       col5
    //履約價格  買賣權別    成交時間    成交價格    成交數量(BorS)
    vector<vector<string>> allData;
    vector<int> timeIdx;
    ReadCsv("cpp_data/2016_01_04.csv", allData, timeIdx);
    int begin = 0;
    //while(begin < allData.size())
    int putCount, callCount;
    putCount = 0;
    callCount = 0;
    int test = 0;
    while(timeIdx.size()>0)
    {
        int end = timeIdx[0];
        timeIdx.erase(timeIdx.begin());
        vector<vector<string>> nowCallPutData(allData.begin()+begin, allData.begin()+end+1);
        vector<string> uniqueCallExercisePrice, uniquePutExercisePrice;
        vector<vector<string>> nowCallData, nowPutData;
        GetUniqueExercisePrice(nowCallPutData, "C", uniqueCallExercisePrice, nowCallData);
        GetUniqueExercisePrice(nowCallPutData, "P", uniquePutExercisePrice, nowPutData);
        /*for(int x=0; x<nowPutData.size();x++)
        {
            for(int y=0; y<nowPutData[x].size(); y++)
            {
                cout << nowPutData[x][y] << "\t";
            }
            cout << endl;
        }*/
        int a, b;
        a = ComputeArbitrage(uniqueCallExercisePrice, nowCallData);
        b = ComputeArbitrage(uniquePutExercisePrice, nowPutData);
        callCount += a;
        putCount += b;
        cout << test << " " << a << " " << b << endl;
        begin = end+1;
        test++;
    }
    cout << callCount << " " << putCount << endl;
    return 0;
}