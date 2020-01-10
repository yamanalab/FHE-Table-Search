#include <iostream>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <ctime>
#include <ratio>
#include <math.h>
#include <cstddef>
#include <iomanip>
#include <string>

using namespace std;

int main(void){
  vector<vector<long> > inputT;
  vector<long> inputT_row, outputT;

  // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  // std::mt19937 generator(seed);

  // for(long i=0 ; i<2 ; ++i){
  //   for(long j=0 ; j<2048 ; ++j){
  //     inputT_row.push_back(j);
  //   }
  //   inputT.push_back(inputT_row);
  //   inputT_row.clear();
  // }

  for(long i=0 ; i<2 ; ++i){
    for(long j=0 ; j<4096 ; ++j){
      if(j<36)
        inputT_row.push_back(j+1);
      else
        inputT_row.push_back(100);
    }
    inputT.push_back(inputT_row);
    inputT_row.clear();
  }
 
  long k=0,d=0;
  for(long i=0 ; i<4096 ; i++){ //the number of possible input values of the first input
    for(long j=0 ; j<4096 ; j++){ //the number of possible input values of the second input
      d++;
      if(i<36 && j<36){
        outputT.push_back(inputT[0][i]+inputT[1][j]);
        k++;
      }
      else
        outputT.push_back(1000);
    }
  }
  cout<<"all output number(not100):"<<k<<",sum num:"<<d<<endl;
  ofstream InputTable;
  InputTable.open("table_in", ios::out);
  for(int i=0 ; i<inputT.size() ; i++){
    for(int j=0 ; j<inputT[0].size() ; j++){
      InputTable << inputT[i][j] << ' ';
    }
    InputTable << endl;
  }
  InputTable.close();

  ofstream OutputTable;
  OutputTable.open("table_out", ios::out);
  for(long i=0 ; i<outputT.size() ; i++){
    OutputTable << outputT[i] << ' ';
  }
  OutputTable << endl;
  OutputTable.close();

  cout << "end" << endl;
  return 0;
}
