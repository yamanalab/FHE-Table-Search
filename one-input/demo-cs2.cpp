//201812@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"
using namespace seal;
using namespace std;

vector<vector <int64_t> > read_table(const string &filename){
  int64_t temp;
	string lineStr;
	vector<vector<int64_t> > table;

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
  int64_t k = ceil(TABLE_SIZE_N/row_size);
  //////////////////////////////////////////////////////////////////////////////

  //read index and PIR query from file
  cout << "Reading query from DS..." << flush;
  string s1(argv[1]);
  fstream PIRqueryFile(s1+'d', fstream::in);
  Ciphertext new_query;
  new_query.load(context, PIRqueryFile);
  PIRqueryFile.close();

  fstream PIRindexFile(s1+'i', fstream::in);
  Ciphertext new_index;
  new_index.load(context, PIRindexFile);
  PIRindexFile.close();
  cout << "OK" << endl;

  //read output table
  vector<vector<int64_t> > LUT_output = read_table(s1+'t');
  vector<Ciphertext> res;
  for(int i=0; i<k ; i++){
    Ciphertext tep;
    res.push_back(tep);
  }
////////////////////////////////////////////////////////////////////
  cout << "threads work" << endl;
  //auto start1=chrono::high_resolution_clock::now();
//#pragma omp parallel for num_threads(NF)
  omp_set_num_threads(NF);
  #pragma omp parallel for
  for (int64_t i=0 ; i<k ; i++){
    LUT_output[i].resize(slot_count);
    Plaintext poly_table_row;
    batch_encoder.encode(LUT_output[i], poly_table_row);
    Ciphertext temp = new_index;
    evaluator.rotate_rows_inplace(temp, -i, gal_keys);
    evaluator.multiply_inplace(temp, new_query);
    evaluator.relinearize_inplace(temp, relin_keys16);
    evaluator.multiply_plain_inplace(temp, poly_table_row);
    evaluator.relinearize_inplace(temp, relin_keys16);
    res[i]=temp;
  }
//////////////////////////////////////////////////////////////////
  //result sum
  Ciphertext sum_result;
  sum_result=res[0];
  for(int i=1 ; i<k ; i++){
    evaluator.add_inplace(sum_result, res[i]);
    evaluator.relinearize_inplace(sum_result, relin_keys16);
  }

  // auto end1=chrono::high_resolution_clock::now();
  // chrono::duration<double> diff = end1-start1;
  // cout << "Select output time is: " << diff.count() << "s" << endl;

  //write Final_result in a file
  cout << "Saving final result..." << flush;
  ofstream Final_result;
  Final_result.open(s1+'r', ios::binary);
  //Final_result << fin_res ;
  sum_result.save(Final_result);
  Final_result.close();
  cout << "OK" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
