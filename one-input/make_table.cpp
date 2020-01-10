#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
//number of columns is a half of slot count!
#define TABLE_SIZE_COL 819200
#define TABLE_SIZE_ROW 2
using namespace std;

int main(void){
  vector<vector<long> > inputTable;
  vector <long> inputTable_row;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator(seed);

  for(int i=0 ; i<TABLE_SIZE_ROW ; i++){
    for(int j=0 ; j<TABLE_SIZE_COL ; j++){
      if(j<36) inputTable_row.push_back(j+1);
      else {
        inputTable_row.push_back(100);
      }
    }
    inputTable.push_back(inputTable_row);
    inputTable_row.clear();
  }

/*
  for(int i=0 ; i<TABLE_SIZE_ROW ; i++){
    for(int j=0 ; j<TABLE_SIZE_COL ; j++){
      if(i==0 && j==0) outputTable_row.push_back(1);
      else {
        int s = (generator()%40+2);
        outputTable_row.push_back(s);
      }
    }
    outputTable.push_back(outputTable_row);
    outputTable_row.clear();
  }
*/
  //output(Table);


  ofstream InputTable;
  InputTable.open("ori_LUT", ios::out);
  for(int i=0 ; i<inputTable.size() ; i++){
    for(int j=0 ; j<inputTable[0].size() ; j++){
      InputTable << inputTable[i][j] << ' ';
    }
    InputTable << endl;
  }
  InputTable.close();

/*
  ofstream OutputTable;
  OutputTable.open("outputTable", ios::out);
  for(int i=0 ; i<outputTable.size() ; i++){
    for(int j=0 ; j<outputTable[0].size() ; j++){
      OutputTable << outputTable[i][j] << ' ';
    }
    OutputTable << endl;
  }
  OutputTable.close();*/
  cout << "end" << endl;
/*
  for (int i=0;i<tot;++i) swap(shlist[i],shlist[i+generator()%(tot-i)]);
  ofstream fshlist("shuffleList.txt");
  for (auto i:shlist) fshlist<<i<<endl;
  fshlist.close();
*/
  return 0;
}
