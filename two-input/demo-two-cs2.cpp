//201812@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"
using namespace seal;
using namespace std;

void out_vector(const vector<int64_t> &a){
  for(int j=0 ; j<a.size() ; j++){

    if(a[j]!= 0) cout<<"\033[1;33m "<<a[j]<<"\033[0m";
    else cout<<a[j]<<" ";
  }
  cout<<endl;
}

vector<vector <int64_t> > read_table(const string &filename){
  int64_t temp;
	string lineStr;
	vector<vector<int64_t> > table;
  cout<<"read table..."<<endl;
  ifstream inFile(filename, ios::in);
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
  cout<<"read table ok"<<endl;
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

  ifstream galFile("GaloisKey");
  GaloisKeys gal_keys;
  gal_keys.unsafe_load(galFile);
  galFile.close();

  ifstream relinFile("RelinKey");
  RelinKeys relin_keys16;
  relin_keys16.unsafe_load(relinFile);
  relinFile.close();

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  BatchEncoder batch_encoder(context);

  size_t slot_count = batch_encoder.slot_count();
  size_t row_size = slot_count / 2;
  cout << "Plaintext matrix row size: " << row_size << endl;
  cout << "Slot nums = " << slot_count << endl;

  int64_t l = row_size;
  int64_t k = ceil(NUM/row_size);
  int64_t ks= ceil(NUM2/row_size);
  //////////////////////////////////////////////////////////////////////////////

  //read index and PIR query from file
  cout << "Reading query from DS..." << flush;
  string s1(argv[1]);
  fstream PIRqueryFile(s1+"_qds2", fstream::in);
  Ciphertext new_query0, new_query1, new_query2;
  new_query0.load(context, PIRqueryFile);
  new_query1.load(context, PIRqueryFile);
  new_query2.load(context, PIRqueryFile);
  PIRqueryFile.close();
  cout<<"query ok"<<endl;


  //read output table
  vector<vector<int64_t> > permute_out = read_table(s1+"_table_out");


  vector<Ciphertext> query_rec, query_sub;
  for(int i=0; i<ks ; i++){
    Ciphertext temp_rec;
    query_rec.push_back(temp_rec);
  }
  for(int i=0; i<row_size ; i++){
    Ciphertext temp_sub;
    query_sub.push_back(temp_sub);
  }

////////////////////////////////////////////////////////////////////
  cout << "First level threads work" << endl;


  omp_set_num_threads(NF);
  #pragma omp parallel for
  for(int64_t i=0 ; i<row_size ; i++){
    Ciphertext temp = new_query2;
    evaluator.rotate_rows_inplace(temp, -i, gal_keys);
    evaluator.multiply_inplace(temp, new_query1);
    evaluator.relinearize_inplace(temp, relin_keys16);
    query_sub[i] = temp;
  }

  cout << "Second level threads work" << endl;
  cout<<"ks:"<<ks<<endl;


  omp_set_num_threads(NF);
  #pragma omp parallel for
  for (int64_t i=0 ; i<ks ; i++){
    int64_t ss = i/row_size;
    int64_t kk = i%row_size;
    permute_out[i].resize(slot_count);
    Plaintext poly_table_row;
    batch_encoder.encode(permute_out[i], poly_table_row);
    Ciphertext temp1 = query_sub[ss];
    evaluator.rotate_rows_inplace(temp1, -kk, gal_keys);
    evaluator.multiply_inplace(temp1, new_query0);
    evaluator.relinearize_inplace(temp1, relin_keys16);
    evaluator.multiply_plain_inplace(temp1, poly_table_row);
    evaluator.relinearize_inplace(temp1, relin_keys16);
    query_rec[i]=temp1;
  }

//////////////////////////////////////////////////////////////////


  cout << "OK" << endl;
  // //result sum
  Ciphertext sum_result;
  sum_result=query_rec[0];
  for(int i=1 ; i<ks ; i++){
    evaluator.add_inplace(sum_result, query_rec[i]);
    evaluator.relinearize_inplace(sum_result, relin_keys16);
  }
  cout << "Size after relinearization: " << sum_result.size() << endl;
  cout << "Noise budget after relinearizing (dbc = "
      << relin_keys16.decomposition_bit_count() << endl;

  //write Final_result in a file
  cout << "Saving final result..." << flush;
  string s2(argv[2]);
  ofstream Final_result;
  Final_result.open(s2, ios::binary);

  sum_result.save(Final_result);
  Final_result.close();
  cout << "OK" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
