#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <string.h>
#include <mpi.h>
using namespace std;

int world_size, world_rank;
MPI_Status status;

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
        int right,left,block;
        block = (allCombination.size()%world_size==0)?(allCombination.size()/world_size):((allCombination.size()/world_size)+1);
        left = world_rank*block;
        right = (world_rank+1)*block;

        for(int x=left; x<right; x++)
        {
            if(x>=allCombination.size())
        	{
        		break;
        	}
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

int main(int argc, char *argv[])
{
    //col1     col2        col3       col4       col5
    //履約價格  買賣權別    成交時間    成交價格    成交數量(BorS)
    MPI_Init(&argc, &argv);    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    vector<vector<string>> allData;
    vector<int> timeIdx;
    DIR *dp;
    struct dirent *dirp;
    string dirname = "cpp_data";
    dp = opendir(dirname.c_str());
    int putCount, callCount, _putCount, _callCount;
    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name, "..") && strcmp(dirp->d_name, "."))
        { 
            string filename;
            filename = dirname + '/' + string(dirp->d_name);
            if(world_rank == 0)
            {
            	cout << filename << ":";
            }   
            ReadCsv(filename, allData, timeIdx);
            int begin = 0;
            _putCount = 0;
            _callCount = 0;
            if(world_rank == 0)
            {
            	cout << filename << ":";
            }   
            while(timeIdx.size()>0)
            {
                int end = timeIdx[0];
                timeIdx.erase(timeIdx.begin());
                vector<vector<string>> nowCallPutData(allData.begin()+begin, allData.begin()+end+1);
                vector<string> uniqueCallExercisePrice, uniquePutExercisePrice;
                vector<vector<string>> nowCallData, nowPutData;
                GetUniqueExercisePrice(nowCallPutData, "C", uniqueCallExercisePrice, nowCallData);
                GetUniqueExercisePrice(nowCallPutData, "P", uniquePutExercisePrice, nowPutData);
                _callCount += ComputeArbitrage(uniqueCallExercisePrice, nowCallData);
                _putCount += ComputeArbitrage(uniquePutExercisePrice, nowPutData);
                begin = end + 1;
            }
            if(world_rank==0)
    		{
		        callCount += _callCount;
		        putCount += _putCount;
		        for(int j=1;j<world_size;j++)
		        {
		            MPI_Recv(&_callCount, 1, MPI_INT, j, j, MPI_COMM_WORLD, &status);
		            MPI_Recv(&_putCount, 1, MPI_INT, j, j, MPI_COMM_WORLD, &status);
		            callCount += _callCount;
		            putCount += _putCount;
		        }
		        cout << callCount << " " << putCount << endl;
		    }
		    else 
		    {
		        MPI_Send(&_callCount, 1, MPI_INT, 0, world_rank, MPI_COMM_WORLD);
		        MPI_Send(&_putCount, 1, MPI_INT, 0, world_rank, MPI_COMM_WORLD);
		    }  
            putCount = 0;
            callCount = 0;          
        	MPI_Barrier(MPI_COMM_WORLD);
        }
        allData.clear();
        timeIdx.clear();     
    }
    MPI_Finalize();
    return 0;
}