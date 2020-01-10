//201905@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"

//when the number of column >= row
using namespace seal;
using namespace std;

vector<int64_t> shift_work(const vector<int64_t> &query, const int64_t &index, const int64_t &num_slots){
  vector<int64_t> new_index;
  int64_t size = query.size();
  int64_t temp;

  for(int64_t i=0 ; i<num_slots ; i++){
    if((i+index) >= size) temp = query[i+index-size];
    else temp=query[i+index];
    new_index.push_back(temp);
  }

  return new_index;
}

void out(vector<int64_t> ss){
  for(int i=0 ; i<ss.size() ; i++){
    cout << ss[i] << " ";
  }cout << endl;
}

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
  cout << "Setting FHE..." << endl;
  ifstream parmsFile("Params");
  EncryptionParameters parms(scheme_type::BFV);
  parms = EncryptionParameters::Load(parmsFile);
  auto context = SEALContext::Create(parms);
  parmsFile.close();

  ifstream skFile("SecretKey");
  SecretKey secret_key;
  secret_key.unsafe_load(skFile);
  skFile.close();

  ifstream pkFile("PublicKey");
  PublicKey public_key;
  public_key.unsafe_load(pkFile);
  pkFile.close();

  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  BatchEncoder batch_encoder(context);
  size_t slot_count = batch_encoder.slot_count();
  size_t row_size = slot_count / 2;
  cout << "Plaintext matrix row size: " << row_size << endl;
  cout << "Slot nums = " << slot_count << endl;

  int64_t l = row_size;
  int64_t k = ceil(TABLE_SIZE_N/row_size);
  //////////////////////////////////////////////////////////////////////////////

  vector<Ciphertext> ct_result;
  vector<vector<int64_t> > dec_result(k);
  vector<Plaintext> poly_dec_result;
  for(int s=0 ; s<k ; s++){
    Plaintext ex;
    poly_dec_result.push_back(ex);
  }
  cout << "Reading intermediate result..." << flush;
  //auto start2=chrono::high_resolution_clock::now();

  string s1(argv[1]);
  ifstream mid_result(s1+'c', ios::binary);
  Ciphertext temp;
  for(int w = 0; w < k ; w++) {
    temp.load(context, mid_result);
    ct_result.push_back(temp);
  }
  mid_result.close();
  cout << "OK" << endl;

  // auto end2=chrono::high_resolution_clock::now();
  // chrono::duration<double> diff2 = end2 - start2;
  // cout << "Reading time is: " << diff2.count() << "s" << endl;

  cout << "Decrypting..."<< flush;

  //auto start1=chrono::high_resolution_clock::now();

//#pragma omp parallel for num_threads(NF)
omp_set_num_threads(NF);
#pragma omp parallel for
  for (int z=0 ; z<k ; z++){
    //cout<<"i: "<<z<<endl;
    decryptor.decrypt(ct_result[z], poly_dec_result[z]);
    batch_encoder.decode(poly_dec_result[z], dec_result[z]);
  }

  //NTL_EXEC_RANGE_END

  cout << "OK" << endl;

  // auto end1=chrono::high_resolution_clock::now();
  // chrono::duration<double> diff = end1-start1;
  // cout << "Decryption time is: " << diff.count() << "s" << endl;

  //output_plaintext(dec_result);
///////////////////////////////////////////////////////////////////
  //find the position of 0.
  cout << "Making PIR-query..." << flush;
  //auto start3=chrono::high_resolution_clock::now();

  int64_t index;
  vector<int64_t> new_query;

  for(int i=0 ; i<k ; i++){
    for(int j=0 ; j<row_size ; j++){
      if(dec_result[i][j] == 0){
        index = i;
        cout<<"i: "<<i<<" j: "<<j<<endl;
        for(int kk=0 ; kk<l ; kk++){
          if(kk==j) new_query.push_back(1);
          else new_query.push_back(0);
        }
      }
    }
  }
  cout << "OK" << endl;

//new_index is new_query left_shift the value of index
  vector<int64_t> new_index;
  //new_query.resize(slot_count);
  new_index = shift_work(new_query,index, row_size);
  cout << "index is "<<index << endl;
  // out(new_index);
  // out(new_query);
  new_query.resize(slot_count);
  new_index.resize(slot_count);

  //encrypt new query
  cout << "Encrypting..." << flush;
  Ciphertext new_PIR_query;
  Plaintext plaintext_new_PIR_query;
  batch_encoder.encode(new_query, plaintext_new_PIR_query);

  Ciphertext new_PIR_index;
  Plaintext plaintext_new_PIR_index;
  batch_encoder.encode(new_index, plaintext_new_PIR_index);

  encryptor.encrypt(plaintext_new_PIR_query, new_PIR_query);
  encryptor.encrypt(plaintext_new_PIR_index, new_PIR_index);

  cout << "OK" << endl;
  // auto end3=chrono::high_resolution_clock::now();
  // chrono::duration<double> diff3 = end3-start3;
  // cout << "Make PIR query time is: " << diff3.count() << "s" << endl;

  //write in a file
  cout << "Saving query..." << flush;
  ofstream PIRqueryFile;
  PIRqueryFile.open(s1+'d', ios::binary);
  new_PIR_query.save(PIRqueryFile);
  PIRqueryFile.close();

  ofstream PIRindexFile;
  PIRindexFile.open(s1+'i', ios::binary);
  new_PIR_index.save(PIRindexFile);
  PIRindexFile.close();

  cout << "OK" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;

}
