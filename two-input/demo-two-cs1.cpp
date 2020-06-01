//201911@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"

using namespace seal;
using namespace std;

void output_plaintext(const vector<vector <int64_t>> &a);
void out_vector(const vector<int64_t> &a);
vector<vector <int64_t> > read_table(const string &filename);
vector<int64_t> read_vector(const string &filename);
vector<int64_t> getRandomVector(int64_t Total);
void Create_LUT(vector<int64_t> &table_x, vector<int64_t> &table_y, vector<int64_t> &table_out, vector<int64_t> vi_x, vector<int64_t> vi_y, vector<vector<int64_t> > &permute_table_x, vector<vector<int64_t> > &permute_table_y, vector<vector<int64_t> > &permute_out, int64_t &l, int64_t &k);

//random seed
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

//output a plaintext table
void output_plaintext(const vector<vector <int64_t>> &a){
  int64_t row=a.size();
  int64_t col=a[0].size();

    for(int i=0 ; i<row ; i++){
        for(int j=0 ; j<col ; j++){
          if(a[i][j]!=100) cout<<"\033[1;34m H"<<a[i][j]<<"\033[0m";
          else cout<<a[i][j]<<" ";
        }
        cout<<endl;
    }
}

void out_vector(const vector<int64_t> &a){
  for(int j=0 ; j<a.size() ; j++){
    //cout<<a[j]<<" ";
    //if(a[j]==2) cout<<"\033[1;33mHere\033[0m";
    if(a[j]!=100) cout<<"\033[1;34m H"<<a[j]<<"\033[0m";
    else cout<<a[j]<<" ";
  }
  cout<<endl;
}

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

vector<int64_t> read_vector(const string &filename){
  int64_t temp;
  string lineStr;
  vector<int64_t> vect;
  ifstream inFile(filename, ios::in);

  while(getline(inFile, lineStr)){
    stringstream ss(lineStr);
    string str;
    while (getline(ss, str, ' ')){
      temp = std::stoi(str);
      vect.push_back(temp);
    }
  }
  return vect;
}

//create a random vector for permutation
// vector<int64_t> getRandomVector(int64_t Total){
//   vector<int64_t> input =* new vector<int64_t>();
//   for(int64_t i=0 ; i<Total ; i++){
//     input.push_back(i);
//   }
//   vector<int64_t> output =* new vector<int64_t>();
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

void Create_LUT(vector<int64_t> &table_x, vector<int64_t> &table_y, vector<int64_t> &table_out, vector<int64_t> vi_x, vector<int64_t> vi_y, vector<vector<int64_t> > &permute_table_x, vector<vector<int64_t> > &permute_table_y, vector<vector<int64_t> > &permute_out, int64_t &l, int64_t &k, int64_t &ks){
  vector<int64_t> per_x, per_y;
  for(int i=0 ; i<NUM ;i++){
    int64_t tempx=vi_x[i];
    int64_t tempy=vi_y[i];
    per_x.push_back(table_x[tempx]);
    per_y.push_back(table_y[tempy]);
  }

  vector<int64_t> sub_per_x, sub_per_y;
  long x_not=0, y_not=0;
  for(int i=0 ; i<k ; i++){
    for(int j=0 ; j<l ; j++){
      sub_per_x.push_back(per_x[i*l+j]);
      sub_per_y.push_back(per_y[i*l+j]);
    }
    permute_table_x.push_back(sub_per_x);
    // out_vector(sub_per_x);
    sub_per_x.clear();
    permute_table_y.push_back(sub_per_y);
    // out_vector(sub_per_y);
    sub_per_y.clear();
  }
  cout<<endl;

  vector<int64_t> per_out, sub_per_out;

  long ooo=0;
  for(int64_t h=0 ; h<NUM ; h++){
    for(int64_t t=0 ; t<NUM ; t++){
      int64_t tepx=vi_x[h];
      int64_t tepy=vi_y[t];

      per_out.push_back(table_out[tepx*NUM+tepy]);
    //   if(table_out[tepx*NUM+tepy]!=1000) ooo++;
    //   if(per_out[h*NUM+t]==2) cout<<"\033[1;32m Here\033[0m"<<endl;
    }
  }
  cout<<"Made permuted output vector is:"<<endl;
  //out_vector(per_out);
  //cout<<"not 1000 has:"<<ooo<<endl;

  cout<<"Permuted out vector"<<endl;
  for(int i=0 ; i<ks ; i++){
    for(int j=0 ; j<l ; j++){
      sub_per_out.push_back(per_out[i*l+j]);
    }
    permute_out.push_back(sub_per_out);
    sub_per_out.clear();
  }

}

vector<int64_t> createrandomvector(int64_t leg){
  vector<int64_t> randomvec;
  for(int i=0 ; i<leg ; ++i){
    int64_t random_value = (generator()%30+1);
    randomvec.push_back(random_value);
  }
  return randomvec;
}


int main(int argc, char *argv[]){
  auto startWhole=chrono::high_resolution_clock::now();
  auto startWhole3=chrono::high_resolution_clock::now();
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
  int64_t k = ceil(NUM/row_size);
  int64_t ks= ceil(NUM2/row_size);
  cout<<"row length:"<<l<<", k:"<<k<<", ks:"<<ks<<endl;
  auto endWhole3=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole3 = endWhole3-startWhole3;

  //////////////////////////////////////////////////////////////////////////////

  //Read query from file
  auto start1=chrono::high_resolution_clock::now();
  cout << "Reading query ..." << flush;
  ifstream readQuery;
  string s1(argv[1]);
  Ciphertext ciphertext_x;
  Ciphertext ciphertext_y;
  readQuery.open(s1, ios::binary);
  ciphertext_x.load(context, readQuery);//input_x
  ciphertext_y.load(context, readQuery);//input_y
  readQuery.close();
  cout << "End" << endl;

  //Read InputTable from file
  cout << "Reading original LUT from file..." << flush;
  vector<vector<int64_t> > table_in=read_table("table_in");
  vector<int64_t> table_out=read_vector("table_out");

  vector<int64_t> table_x, table_y, table_output;
  for(int i=0 ; i<NUM ; ++i){
    table_x.push_back(table_in[0][i]);
    table_y.push_back(table_in[1][i]);
  }
  cout << "OK" << endl;


  vector<int64_t> vi_x = getRandomVector(NUM);
  vector<int64_t> vi_y = getRandomVector(NUM);

  //change all position, make new LUT
  cout << "Make new LUT ..." << flush;
  vector<vector<int64_t> > permute_table_x, permute_table_y, permute_out;
  Create_LUT(table_x, table_y, table_out, vi_x, vi_y, permute_table_x, permute_table_y, permute_out, l, k, ks);//
  cout<<"end"<<endl;

  //write shifted_output_table in a file
  ofstream OutputTable;
  OutputTable.open(s1+"_table_out");//OutputTable
  for(int i=0 ; i<ks ; i++){
    for(int j=0 ; j<l ; j++){
      OutputTable << permute_out[i][j] <<' ';
    }
    OutputTable << endl;
  }
  OutputTable.close();

  auto end1=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole1 = end1-start1;

  //Make Threads to compute every row of table
  auto startWhole2=chrono::high_resolution_clock::now();
  cout << "Make threads for each row and compute" << endl;
  vector<Ciphertext> result_x, result_y;
  for(int i=0; i<k ; i++){
    Ciphertext tep;
    result_x.push_back(tep);
    result_y.push_back(tep);
  }

  //thread work
  omp_set_num_threads(NF);
  #pragma omp parallel for
  for(int64_t i=0 ; i<k ; i++){
    Ciphertext res_x = ciphertext_x;
    Plaintext poly_row_x;
    permute_table_x[i].resize(slot_count);
    //out_vector(permute_table_x[i]);
    batch_encoder.encode(permute_table_x[i], poly_row_x);
    evaluator.sub_plain_inplace(res_x, poly_row_x);
    evaluator.relinearize_inplace(res_x, relin_keys16);

    vector<int64_t> random_value_vec1 = createrandomvector(slot_count);
    Plaintext poly_num_x;
    batch_encoder.encode(random_value_vec1, poly_num_x);

    evaluator.multiply_plain_inplace(res_x, poly_num_x);
    evaluator.relinearize_inplace(res_x, relin_keys16);
    cout << "Size after relinearization: " << res_x.size() << endl;
    cout << "Noise budget after relinearizing (dbc = "
        << relin_keys16.decomposition_bit_count() << endl;
    result_x[i]=res_x;

    Ciphertext res_y = ciphertext_y;
    Plaintext poly_row_y;
    permute_table_y[i].resize(slot_count);
    //out_vector(permute_table_y[i]);
    batch_encoder.encode(permute_table_y[i], poly_row_y);
    evaluator.sub_plain_inplace(res_y, poly_row_y);
    evaluator.relinearize_inplace(res_y, relin_keys16);

    vector<int64_t> random_value_vec2 = createrandomvector(slot_count);
    Plaintext poly_num_y;
    batch_encoder.encode(random_value_vec2, poly_num_y);

    evaluator.multiply_plain_inplace(res_y, poly_num_y);
    evaluator.relinearize_inplace(res_y, relin_keys16);
    cout << "Size after relinearization: " << res_y.size() << endl;
    cout << "Noise budget after relinearizing (dbc = "
        << relin_keys16.decomposition_bit_count() << endl;
    result_y[i]=res_y;
  }
  // auto end1=chrono::high_resolution_clock::now();
  // chrono::duration<double> diffWhole1 = end1-start1;
  // cout << "Without file reading runtime is: " << diffWhole1.count() << "s" << endl;

  //Get vector<Ctxt> Result
  cout << "Saving..." << flush;
  ofstream outResult_x, outResult_y;
  outResult_x.open(s1+"_xc1", ios::binary);
  outResult_y.open(s1+"_yc1", ios::binary);
  for(int i=0 ; i<k ; i++){
    result_x[i].save(outResult_x);
    result_y[i].save(outResult_y);
  }
  outResult_x.close();
  outResult_y.close();
  cout << "OK" << endl;
  auto endWhole2=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole2 = endWhole2-startWhole2;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "set FHE runtime is: " << diffWhole3.count() << "s" << endl;
  cout << "construct LUT runtime is: " << diffWhole1.count() << "s" << endl;
  cout << "thread work runtime is: " << diffWhole2.count() << "s" << endl;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
