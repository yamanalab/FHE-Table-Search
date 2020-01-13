//This code makes a demo table, not for real function
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

  ofstream InputTable;
  InputTable.open("ori_LUT", ios::out);
  for(int i=0 ; i<inputTable.size() ; i++){
    for(int j=0 ; j<inputTable[0].size() ; j++){
      InputTable << inputTable[i][j] << ' ';
    }
    InputTable << endl;
  }
  InputTable.close();

  cout << "end" << endl;

  return 0;
}
