//201811@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"

using namespace seal;
using namespace std;
//random seed
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

//read table from file
vector<vector <int64_t> > read_table(const string &filename){
  int64_t temp;
	string lineStr;
	vector<vector<int64_t> > table;

  ifstream inFile(filename, ios::in);//read from
	while (getline(inFile, lineStr)){
    vector<int64_t> table_col;
		stringstream ss(lineStr);
		string str;
		while (getline(ss, str, ' ')){
      temp = std::stoi(str);
			table_col.push_back(temp);
    }
		table.push_back(table_col);
	}
  return table;
}

//create a random vector for permutation
// vector<int64_t> getRandomVector(int64_t Total){
//   vector<int64_t> input =* new vector<int64_t>();
//   for(int64_t i=0 ; i<Total ; i++){
//     input.push_back(i);
//   }
//   vector<int64_t> output=*new vector<int64_t>();
//   int end=Total;
//   for(int64_t i=0;i<Total;i++){
//     vector<int64_t>::iterator iter=input.begin();
//     int64_t num=generator()%end;
//     iter+=num;
//     output.push_back(*iter);
//     input.erase(iter);
//     end--;
//   }
//   return output;
// }
bool cmp(pair<int64_t, int64_t> a, pair<int64_t, int64_t> b){
  return a.second < b.second;
}

vector<int64_t> getRandomVector(int64_t Total){
  vector<pair<int64_t, int64_t> > input;
  vector<int64_t> output;
  for(int64_t i=0 ; i<Total ; i++){
    input.push_back({i, generator()%1000000});
  }
  sort(input.begin(), input.end(), cmp);
  for(int64_t i=0 ; i<Total ; i++){
    output.push_back(input[i].first);
  }
  return output;
}
//creat new LUT
void Create_LUT(vector<vector<int64_t> > LUT, vector<int64_t> randomVector,vector<vector<int64_t> > &LUT_input, vector<vector<int64_t> > &LUT_output, int64_t &l, int64_t &k){
  cout<<k<<' '<<l<<endl;
  vector<int64_t> sub_input;
  vector<int64_t> sub_output;
  int64_t Total=randomVector.size();
  cout<<Total<<endl;
  int row_size=l;
  int64_t index=0;
    for(int64_t i=0 ; i<k ; i++){
      for(int64_t j=0 ; j<row_size ; j++){
        int64_t s=randomVector[index];
        int64_t temp_in=LUT[0][s];
        sub_input.push_back(temp_in);
        int64_t temp_out=LUT[1][s];
        sub_output.push_back(temp_out);
        index++;
      }
      sub_input.resize(l);
      LUT_input.push_back(sub_input);
      sub_input.clear();
      sub_output.resize(l);
      LUT_output.push_back(sub_output);
      sub_output.clear();
    }
    cout<<"end"<<endl;
}

//output a plaintext table
void output_plaintext(const vector<vector <int64_t>> &a){
  int64_t row=a.size();
  int64_t col=a[0].size();

    for(int i=0 ; i<row ; i++){
        for(int j=0 ; j<col ; j++)
            cout<<a[i][j]<<" ";
        cout<<endl;
    }
}

void out_vector(vector<int64_t> x){
  for(int i=0 ; i<x.size() ; ++i){
    cout<<x[i]<<" ";
  }
}

int main(int argc, char *argv[]){
  auto startWhole=chrono::high_resolution_clock::now();
  //resetting FHE
  cout << "Setting FHE..." << flush;
  ifstream parmsFile("Params");
  EncryptionParameters parms(scheme_type::BFV);
  parms = EncryptionParameters::Load(parmsFile);
  auto context = SEALContext::Create(parms);
  parmsFile.close();

  ifstream pkFile("PublicKey");
  PublicKey public_key;
  public_key.unsafe_load(pkFile);
  pkFile.close();
  //KeyGenerator keygen(context);
  //PublicKey public_key = keygen.public_key();
  ifstream relinFile("RelinKey");
  RelinKeys relin_keys16;
  relin_keys16.unsafe_load(relinFile);
  relinFile.close();

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);

  BatchEncoder batch_encoder(context);
  IntegerEncoder encoder(context);
  size_t slot_count = batch_encoder.slot_count();
  size_t row_size = slot_count / 2;
  cout << "Plaintext matrix row size: " << row_size << endl;
  cout << "Slot nums = " << slot_count << endl;

  int64_t l = row_size;
  int64_t k = ceil(TABLE_SIZE_N/row_size);
  //////////////////////////////////////////////////////////////////////////////

auto startWhole1=chrono::high_resolution_clock::now();
  //Read query from file
  cout << "Reading query ..." << flush;
  ifstream readQuery;
  string s1(argv[1]);
  Ciphertext ciphertext_query;
  readQuery.open(s1, ios::binary);
  ciphertext_query.load(context, readQuery);
  readQuery.close();
  cout << "End" << endl;


  //Read InputTable from file
  cout << "Reading original LUT from file..." << flush;
  vector<vector<int64_t> > ori_LUT = read_table("ori_LUT");
  cout << "OK" << endl;

  vector<int64_t> vi = getRandomVector(TABLE_SIZE_N);
 // for(int i=0 ; i<TABLE_SIZE_N ; i++){
 //   cout << vi[i] <<' '<< i<<endl;
 // }
 // cout<<endl;

  //auto start2=chrono::high_resolution_clock::now();
  //change all position, make new LUT
  cout << "Make new LUT ..." << flush;
  vector<vector<int64_t> > LUT_input;
  vector<vector<int64_t> > LUT_output;
  Create_LUT(ori_LUT, vi, LUT_input, LUT_output, l, k);
  cout<<"end"<<endl;

  //auto end2=chrono::high_resolution_clock::now();
  //chrono::duration<double> diff2 = end2-start2;
  //cout << "Construct LUTs time is: " << diff2.count() << "s" << endl;

  //write shifted_output_table in a file
  ofstream OutputTable;
  OutputTable.open(s1+'t');
  for(int i=0 ; i<k ; i++){
    for(int j=0 ; j<l ; j++){
      OutputTable << LUT_output[i][j] <<' ';
    }
    OutputTable << endl;
  }
  OutputTable.close();

  auto endWhole1=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole1 = endWhole1-startWhole1;


auto startWhole2=chrono::high_resolution_clock::now();
  //Make Threads to compute every row of table
  cout << "Make threads for each row and compute" << endl;
  vector<Ciphertext> Result;

  for(int i=0; i<k ; i++){
    Ciphertext tep;
    Result.push_back(tep);
  }
  //thread work
  //auto start1=chrono::high_resolution_clock::now();

  //NTL::SetNumThreads(NF);

  //NTL_EXEC_RANGE(k, first, last)
//#pragma omp parallel for num_threads(NF)
omp_set_num_threads(NF);
#pragma omp parallel for
for(int64_t i=0 ; i<k ; i++){
  //LUT_input[i].resize(slot_count);
  Ciphertext res = ciphertext_query;
  Plaintext poly_row;
  batch_encoder.encode(LUT_input[i], poly_row);
  evaluator.sub_plain_inplace(res, poly_row);
  evaluator.relinearize_inplace(res, relin_keys16);

  vector<int64_t> random_value_vec;
  for(int64_t sk=0 ; sk<row_size ; ++sk){
    int64_t random_value=(generator()%5+1);
    random_value_vec.push_back(random_value);
  }
  random_value_vec.resize(slot_count);
  Plaintext poly_num;
  batch_encoder.encode(random_value_vec, poly_num);
  // int64_t random_value=(generator()%5+1);
  // Plaintext poly_num = encoder.encode(random_value);

  evaluator.multiply_plain_inplace(res, poly_num);
  evaluator.relinearize_inplace(res, relin_keys16);
  Result[i]=res;
}

  //NTL_EXEC_RANGE_END

  auto endWhole2=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole2 = endWhole2-startWhole2;
  //Get vector<Ctxt> Result
  cout << "Saving..." << flush;
  //save in a file
  ofstream outResult;
  outResult.open(s1+'c', ios::binary);
  for(int i=0 ; i<k ; i++){
    Result[i].save(outResult);
    //outResult<<endl;
  }
  outResult.close();
  cout << "OK" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "construct table time: " << diffWhole1.count() << "s" << endl;
  cout << "thread work time: " << diffWhole2.count() << "s" << endl;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
