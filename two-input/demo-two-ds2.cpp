//201911@Richelle
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

  void out_vector(const vector<int64_t> &a){
    for(int j=0 ; j<a.size() ; j++){
      //cout<<a[j]<<" ";
      //if(a[j]==2) cout<<"\033[1;33mHere\033[0m";
      if(a[j]!=0) cout<<"\033[1;34m H"<<a[j]<<"j:"<<j<<"\033[0m";
      else cout<<a[j]<<" ";
    }
    cout<<endl;
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
  cout << "\033[1;31mSetting FHE...\033[0m" << endl;
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
  cout << "\033[1;31mPlaintext matrix row size: \033[0m" << row_size << endl;
  cout << "\033[1;31mSlot nums = \033[0m" << slot_count << endl;

  int64_t l = row_size;
  int64_t k = ceil(NUM/row_size);
  int64_t ks= ceil(NUM2/row_size);
  cout<<"row length:"<<l<<", k:"<<k<<", ks:"<<ks<<endl;
  //////////////////////////////////////////////////////////////////////////////

  vector<Ciphertext> ct_result_x;
  vector<vector<int64_t> > dec_result_x(k);
  vector<Plaintext> poly_dec_result_x;
  for(int s=0 ; s<k ; s++){
    Plaintext ex_x;
    poly_dec_result_x.push_back(ex_x);
  }

  vector<Ciphertext> ct_result_y;
  vector<vector<int64_t> > dec_result_y(k);
  vector<Plaintext> poly_dec_result_y;
  for(int s=0 ; s<k ; s++){
    Plaintext ex_y;
    poly_dec_result_y.push_back(ex_y);
  }
  cout << "\033[1;31mReading intermediate result...\033[0m" << flush;

  string s1(argv[1]);
  ifstream mid_result_x(s1+"_xc1", ios::binary);
  Ciphertext temp_x;
  for(int w = 0; w < k ; w++) {
    temp_x.load(context, mid_result_x);
    ct_result_x.push_back(temp_x);
  }
  mid_result_x.close();

  ifstream mid_result_y(s1+"_yc1", ios::binary);
  Ciphertext temp_y;
  for(int w = 0; w < k ; w++) {
    temp_y.load(context, mid_result_y);
    ct_result_y.push_back(temp_y);
  }
  mid_result_y.close();
  cout << "\033[1;31mOK\033[0m" << endl;
  cout << "\033[1;31mDecrypting...\033[0m"<< flush;

  omp_set_num_threads(NF);
  #pragma omp parallel for
  for (int z=0 ; z<k ; z++){
    decryptor.decrypt(ct_result_x[z], poly_dec_result_x[z]);
    batch_encoder.decode(poly_dec_result_x[z], dec_result_x[z]);
  }

  omp_set_num_threads(NF);
  #pragma omp parallel for
  for (int z=0 ; z<k ; z++){
    decryptor.decrypt(ct_result_y[z], poly_dec_result_y[z]);
    batch_encoder.decode(poly_dec_result_y[z], dec_result_y[z]);
  }
  //NTL_EXEC_RANGE_END
  // cout << "\033[1;31m decrypted x result\033[0m" << endl;
  // for(int i=0 ; i<k ; i++){
  //     for(int j=0 ; j<l ; j++){
  //       cout<<dec_result_x[i][j]<<" ";
  //       if(dec_result_x[i][j]==0) cout<<"\033[1;33mZero is in here\033[0m";
  //     }
  //     cout<<endl;
  // }
  // cout << "\033[1;31m decrypted y result\033[0m" << endl;
  // for(int i=0 ; i<k ; i++){
  //     for(int j=0 ; j<l ; j++){
  //       cout<<dec_result_y[i][j]<<" ";
  //       if(dec_result_y[i][j]==0) cout<<"\033[1;32mZero is in here\033[0m";
  //     }
  //     cout<<endl;
  // }

  cout << "\033[1;31mOK\033[0m" << endl;
  //output_plaintext(dec_result);
///////////////////////////////////////////////////////////////////
  //find the position of 0.
  cout << "\033[1;31mMaking PIR-query...\033[0m" << flush;

  int64_t index_row_x, index_col_x;
  for(int64_t i=0 ; i<k ; i++){
    for(int64_t j=0 ; j<row_size ; j++){
      //cout<<"xi:"<<i<<", xj:"<<j<<", res:"<<dec_result_x[i][j]<<endl;
      //cout<<"xi:"<<i<<"xj:"<<j<<endl;
      if(dec_result_x[i][j] == 0){
        index_row_x = i;
        index_col_x = j;
        //cout<<"\033[1;31mxi:\033[0m"<<i<<"\033[1;31mxj:\033[0m"<<j<<endl;
      }
    }
  }
  //cout<<"oooo"<<endl;
  vector<int64_t> new_query0, new_query1, new_query2;
  int64_t index_row_y, index_col_y;
  for(int64_t i=0 ; i<k ; i++){
    for(int64_t j=0 ; j<row_size ; j++){
      //cout<<"yi:"<<i<<"yj:"<<j<<endl;
      if(dec_result_y[i][j] == 0){
        index_row_y = i;
        index_col_y = j;
        //cout<<"yi:"<<i<<"yj:"<<j<<endl;
        for(int kk=0 ; kk<l ; kk++){
          if(kk==j) new_query0.push_back(1);
          else new_query0.push_back(0);
        }
      }
    }
  }

  cout<<"index_row_x:"<<index_row_x<<", index_col_x:"<<index_col_x<<", index_row_y:"<<index_row_y<<", index_col_y:"<<index_col_y<<endl;
  //out_vector(new_query0);

  cout << "\033[1;31mOK\033[0m" << endl;

//new_index is new_query left_shift the value of index
  int64_t index1 = (l*index_row_x+index_col_x)*k+index_row_y;
  index1 = index1%l;
  int64_t index2 = (l*index_row_x+index_col_x)*k+index_row_y;
  index2 = index2/l;

  new_query1 = shift_work(new_query0 ,index1, row_size);
  new_query2 = shift_work(new_query1, index2, row_size);
  cout << "\033[1;31mindex1 is \033[0m"<<index1 << endl;
  cout << "\033[1;31mindex2 is \033[0m"<<index2 << endl;
  //out_vector(new_query1);
  //out_vector(new_query2);
  new_query0.resize(slot_count);
  new_query1.resize(slot_count);
  new_query2.resize(slot_count);
  //out_vector(new_query1);
  //out_vector(new_query2);
  // out(new_index);
  // out(new_query);

  //encrypt new query
  cout << "\033[1;31mEncrypting...\033[0m" << flush;
  Ciphertext new_PIR_query0, new_PIR_query1, new_PIR_query2;
  Plaintext plaintext_new_PIR_query0, plaintext_new_PIR_query1, plaintext_new_PIR_query2;
  batch_encoder.encode(new_query0, plaintext_new_PIR_query0);
  encryptor.encrypt(plaintext_new_PIR_query0, new_PIR_query0);
  batch_encoder.encode(new_query1, plaintext_new_PIR_query1);
  encryptor.encrypt(plaintext_new_PIR_query1, new_PIR_query1);
  batch_encoder.encode(new_query2, plaintext_new_PIR_query2);
  encryptor.encrypt(plaintext_new_PIR_query2, new_PIR_query2);
  cout << "\033[1;31mOK\033[0m" << endl;

  //write in a file
  cout << "\033[1;31mSaving query...\033[0m" << flush;
  ofstream queryFile;
  queryFile.open(s1+"_qds2", ios::binary);
  new_PIR_query0.save(queryFile);
  new_PIR_query1.save(queryFile);
  new_PIR_query2.save(queryFile);
  queryFile.close();
  cout << "\033[1;31mOK\033[0m" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
