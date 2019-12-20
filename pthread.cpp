#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
using namespace std;



typedef struct _thread_data_t {
    int begin;
    int end;
} threadData;
vector<string> allFileName;
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
            for(int i=0; i<nowData.size(); i++)
            {
                if(stoi(nowData[i][0]) == exercisePrice[0]) dataIdx1.push_back(i);
                if(stoi(nowData[i][0]) == exercisePrice[1]) dataIdx2.push_back(i);
                if(stoi(nowData[i][0]) == exercisePrice[2]) dataIdx3.push_back(i);
            }
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
                        if(tmp1 > stoi(nowData[dataIdx1[a]][4]) || tmp2 > stoi(nowData[dataIdx3[c]][4]) || tmp3 > stoi(nowData[dataIdx2[b]][4]))
                        {
                            continue;
                        }
                        if(tmp1*stof(nowData[dataIdx1[a]][3])+tmp2*stof(nowData[dataIdx3[c]][3])-tmp3*stof(nowData[dataIdx2[b]][3]) >= 20*(tmp1+tmp2+tmp3))
                        {
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


void *worker(void *arg)
{
    threadData data = *(threadData *)arg;
    for(int xx=data.begin; xx<data.end; xx++)
    {
        int putCount=0, callCount=0;
        vector<int> timeIdx;
        timeIdx.push_back(0);
        vector<vector<string>> allData;
        ReadCsv(allFileName[xx], allData, timeIdx);
        for(int i=0; i<timeIdx.size()-1; i++)
        {
            int begin;
            if(i == 0) begin = timeIdx[i];
            else begin = timeIdx[i]+1;
            int end = timeIdx[i+1];
            vector<vector<string>> nowCallPutData(allData.begin()+begin, allData.begin()+end+1);
            vector<string> uniqueCallExercisePrice, uniquePutExercisePrice;
            vector<vector<string>> nowCallData, nowPutData;
            GetUniqueExercisePrice(nowCallPutData, "C", uniqueCallExercisePrice, nowCallData);
            GetUniqueExercisePrice(nowCallPutData, "P", uniquePutExercisePrice, nowPutData);
            int a, b;
            a = ComputeArbitrage(uniqueCallExercisePrice, nowCallData);
            b = ComputeArbitrage(uniquePutExercisePrice, nowPutData);
            callCount += a;
            putCount += b;
        }
        cout << allFileName[xx] << ":" << callCount << "," << putCount << endl;
    }
}


int main()
{
    //col1     col2        col3       col4       col5
    //履約價格  買賣權別    成交時間    成交價格    成交數量(BorS)
    int threadCount = 4;
    DIR *dp;
    struct dirent *dirp;
    string dirname = "cpp_data";
    dp = opendir(dirname.c_str());
    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name, "..") && strcmp(dirp->d_name, "."))
        { 
            allFileName.push_back(dirname + '/' + string(dirp->d_name));
        }
    }
            
    int divideCount;
    divideCount = allFileName.size()/threadCount;
    pthread_t threadArr[threadCount];
    threadData thData[threadCount];
    for(int x=0; x<threadCount; x++)
    {
        int end;
        if(x == threadCount-1) thData[x].end = allFileName.size();
        else thData[x].end = (x+1)*divideCount;
        thData[x].begin = x*divideCount;
        //vector<string> tmp(allFileName.begin()+(x*divideCount), allFileName.begin()+end);
        //thData.fileName = tmp;
        pthread_create(&threadArr[x], NULL, worker, (void *)&thData[x]);
    }
    for(int i=0; i<threadCount; i++) {
        pthread_join(threadArr[i], NULL);
    }
    return 0;
}