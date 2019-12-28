#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

using namespace std;
#define MAXROWCOUNT 1000000
float* allData;
int* timeIdx;
int rowCount=0, timeCount=1;
int ans;
pthread_mutex_t mutex;



typedef struct _thread_data_t {
    int begin;
    int end;
    float* uniqueExercisePrice;
    int* allCombination;
    int* size;
    float* nowData;
} threadData;


void ReadCsv(string fileName)
{
    allData = new float[MAXROWCOUNT*5];
    timeIdx = new int[MAXROWCOUNT];
    timeIdx[0] = 0;
    fstream file;
    file.open(fileName);
    rowCount=0;
    timeCount=1;
    string line;
    string timeStart = "84500";
    while (getline(file, line, '\n'))
    {
        istringstream templine(line); 
        string data;
        int checkTime = 0;
        while (getline(templine, data, ',')) 
        {
            if(checkTime == 2 && data != timeStart)
            {
                timeIdx[timeCount] = rowCount-1;
                timeStart = data;
                timeCount++;
            }
            if(checkTime == 1)
            {
                if(data == "C") allData[rowCount*5+checkTime] = 1.0;
                if(data == "P") allData[rowCount*5+checkTime] = -1.0;
            }
            else
            {
                allData[rowCount*5+checkTime] = atof(data.c_str());
            }
            checkTime++;
        }
        rowCount++;
    }
    file.close();
}

int* GetUniqueExercisePrice(float* allData, int begin, int end, float cpType, float *uniqueExercisePrice, float *nowData)
{
    int uniqueExercisePriceLen = 0;
    int nowDataLen = 0;
    for(int i=begin; i<=end; i++)
    {
        if(allData[i*5+1] == cpType)
        {
            for(int j=0;j<5;j++)
            {
                nowData[nowDataLen] = allData[i*5+j];
                nowDataLen++;
            }
            int z=0;
            for(z=0;z<uniqueExercisePriceLen;z++)
            {
                if(uniqueExercisePrice[z] == allData[i*5+0]) break;
            }
            if(z == uniqueExercisePriceLen)
            {
                uniqueExercisePrice[uniqueExercisePriceLen] = allData[i*5+0];
                uniqueExercisePriceLen++;
            }
        }

    }
    int* ans;
    ans = new int[2];
    ans[0] = uniqueExercisePriceLen;
    ans[1] = nowDataLen/5;
    return ans;
}


void Combination(int *arr, int *data, int start, int end, int index, int r, int *ans) 
{ 
    // Current combination is ready to be printed, print it 
    if (index == r) 
    { 
        for (int j=0; j<r; j++) {
            ans[ans[99999]*r+j] = data[j];
        }
        ans[99999]++; 
        return; 
    } 
    for (int i=start; i<=end && end-i+1 >= r-index; i++) 
    { 
        data[index] = arr[i]; 
        Combination(arr, data, i+1, end, index+1, r, ans); 
    } 
} 


int gcd(int a, int b)
{
    while (a != b)
    {
        if (a > b)
        {
            return gcd(a - b, b);
        }
        else
        {
            return gcd(a, b - a);
        }
    }
    return a;
}

void *worker(void *arg)
{
    threadData data = *(threadData *)arg;

    for(int x=data.begin; x<data.end; x++)
    {
        float* exercisePrice;
        exercisePrice = new float[3];
        int* dataIdx1;
        int* dataIdx2;
        int* dataIdx3;
        int data1Count=0, data2Count=0, data3Count=0;
        dataIdx1 = new int[1000];
        dataIdx2 = new int[1000];
        dataIdx3 = new int[1000];
        exercisePrice[0] = data.uniqueExercisePrice[data.allCombination[x*3+0]];
        exercisePrice[1] = data.uniqueExercisePrice[data.allCombination[x*3+1]];
        exercisePrice[2] = data.uniqueExercisePrice[data.allCombination[x*3+2]];
        for(int i=0; i<data.size[1]; i++)
        {
            if(data.nowData[i*5+0] == exercisePrice[0]) 
            {
                dataIdx1[data1Count] = i;
                data1Count++;
            }
            if(data.nowData[i*5+0] == exercisePrice[1])
            {
                dataIdx2[data2Count] = i;
                data2Count++;
            }
            if(data.nowData[i*5+0] == exercisePrice[2])
            {
                dataIdx3[data3Count] = i;
                data3Count++;

            }
        }
        for(int a=0; a<data1Count; a++)
        {
            for(int b=0; b<data2Count; b++)
            {
                for(int c=0; c<data3Count; c++)
                {
                    int tmp1, tmp2, tmp3, tmpGcd;
                    tmp1 = exercisePrice[2] - exercisePrice[1];
                    tmp2 = exercisePrice[1] - exercisePrice[0];
                    tmp3 = exercisePrice[2] - exercisePrice[0];
                    tmpGcd = gcd(gcd(tmp1, tmp2), tmp3);
                    //cout << exercisePrice[2] << " " << exercisePrice[1] << " " << exercisePrice[0] << endl;
                    tmp1 /= tmpGcd;
                    tmp2 /= tmpGcd;
                    tmp3 /= tmpGcd;
                    //cout << a << " " << b << " " << c << " " << data1Count << " " << data2Count << " " << data3Count << endl;
                    //cout << tmp1 << " " << tmp2 << " " << tmp3 << endl;
                    if(tmp1 > data.nowData[dataIdx1[a]*5+4] || tmp2 > (data.nowData[dataIdx3[c]*5+4]) || tmp3 > (data.nowData[dataIdx2[b]*5+4]))
                    {
                        continue;
                    }
                    if(tmp1*(data.nowData[dataIdx1[a]*5+3])+tmp2*(data.nowData[dataIdx3[c]*5+3])-tmp3*(data.nowData[dataIdx2[b]*5+3]) >= 20*(tmp1+tmp2+tmp3))
                    {
                        pthread_mutex_lock(&mutex);
                        ans++;
                        pthread_mutex_unlock(&mutex);
                    }
                }
            }
        }
        delete [] dataIdx1;
        delete [] dataIdx2;
        delete [] dataIdx3;
    }
    pthread_exit(NULL);
}



int ComputeArbitrage(float* uniqueExercisePrice, float* nowData, int* size, int threadCount)
{
    ans = 0;
    if(size[0] >= 3)
    {
        int* allCombination;
        int combinationCount;
        allCombination = new int[100000];
        allCombination[99999] = 0;
        int *data, *arr;
        data = new int[100000];
        arr = new int[100000];
        for(int i=0;i<size[0];i++) arr[i] = i;
        Combination(arr, data, 0, size[0]-1, 0, 3, allCombination);
        delete[] data;
        delete[] arr;
        //cout << size[0] << endl;
        //cout << combinationCount << endl;
        if(allCombination[99999] <= threadCount)
        {
            pthread_t threadArr[allCombination[99999]];
            threadData thData[allCombination[99999]];
            for(int x=0; x<allCombination[99999]; x++)
            {
                thData[x].end = (x+1);
                thData[x].begin = x;
                thData[x].uniqueExercisePrice = uniqueExercisePrice;
                thData[x].allCombination = allCombination;
                thData[x].size = size;
                thData[x].nowData = nowData;
                pthread_create(&threadArr[x], NULL, worker, (void *)&thData[x]);
            }
            for(int i=0; i<allCombination[99999]; i++) {
                pthread_join(threadArr[i], NULL);
            }
        }
        else
        {
            int divideCount;
            divideCount = allCombination[99999]/threadCount;
            pthread_t threadArr[threadCount];
            threadData thData[threadCount];
            for(int x=0; x<threadCount; x++)
            {
                int end;
                if(x == threadCount-1) thData[x].end = allCombination[99999];
                else thData[x].end = (x+1)*divideCount;
                thData[x].begin = x*divideCount;
                thData[x].uniqueExercisePrice = uniqueExercisePrice;
                thData[x].allCombination = allCombination;
                thData[x].size = size;
                thData[x].nowData = nowData;
                pthread_create(&threadArr[x], NULL, worker, (void *)&thData[x]);
            }
            for(int i=0; i<threadCount; i++) {
                pthread_join(threadArr[i], NULL);
            }
        }
        return ans;
    }
    else
    {
        return 0;
    }
}

int main(int argc, char *argv[])
{
    //col1     col2        col3       col4       col5
    //履約價格  買賣權別    成交時間    成交價格    成交數量(BorS)
    pthread_mutex_init(&mutex, 0);
    DIR *dp;
    struct dirent *dirp;
    string dirname = "cpp_data";
    dp = opendir(dirname.c_str());
    int putCount, callCount;
    int fileCount = 0;
    int threadCount = atoi(argv[2]);

    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name, "..") && strcmp(dirp->d_name, "."))
        { 
            string filename;
            filename = dirname + '/' + string(dirp->d_name);
            cout << filename << ":"; 
            ReadCsv(filename);
            putCount = 0;
            callCount = 0;
            for(int i=0; i<timeCount-1; i++)
            {
                int begin;
                if(i == 0) begin = timeIdx[i];
                else begin = timeIdx[i]+1;
                int end = timeIdx[i+1];
                
                float *uniqueCallExercisePrice;
                float *uniquePutExercisePrice;
                float *nowCallData;
                float *nowPutData;
                uniqueCallExercisePrice = new float[1000];
                uniquePutExercisePrice = new float[1000];
                nowCallData = new float[10000*5];
                nowPutData = new float[10000*5];
                int *a;
                int *b;
                a = new int[2];
                b = new int[2];
                a = GetUniqueExercisePrice(allData, begin, end, 1.0, uniqueCallExercisePrice, nowCallData);
                b = GetUniqueExercisePrice(allData, begin, end, -1.0, uniquePutExercisePrice, nowPutData);
                //cout << begin << "->" << end << ":" << b[0] << " " << b[1] << endl;
                callCount += ComputeArbitrage(uniqueCallExercisePrice, nowCallData, a, threadCount);
                putCount += ComputeArbitrage(uniquePutExercisePrice, nowPutData, b, threadCount);
                delete[] uniqueCallExercisePrice;
                delete[] nowCallData;
                delete[] uniquePutExercisePrice;
                delete[] nowPutData;
                delete[] a;
                delete[] b;
            }
            cout << callCount << " " << putCount << endl;
            fileCount++;
        }
        delete[] allData;
        delete[] timeIdx;
        if(fileCount == atoi(argv[1])) break;
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}