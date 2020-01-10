//201911@Richelle
//୧(๑•̀⌄•́๑)૭✧

#include "server.hpp"

using namespace seal;
using namespace std;

void out(vector<int64_t> ss){
  for(int i=0 ; i<ss.size() ; i++){
    cout << ss[i] << " ";
  }cout << endl;
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
  cout<<"row length:"<<l<<", k:"<<k<<", ks:"<<ks<<endl;
/////////////////////////////////////////////////////////////////////////////////////////////////
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator(seed);

  //make query x and y
  cout<<"Get query!: ";
  //int64_t query_x=generator()%59+1;
  int64_t query_x=1;
  cout<<query_x<<endl;
  //int64_t query_y=generator()%59+1;
  int64_t query_y=1;
  cout<<query_y<<endl;

  //encrypt the query
  cout << "Encrypt and save your query..." << flush;
  vector<int64_t> vec_x;
  for(int i=0 ; i<row_size ; i++){
    vec_x.push_back(query_x);
  }
  vec_x.resize(slot_count);

  vector<int64_t> vec_y;
  for(int i=0 ; i<row_size ; i++){
    vec_y.push_back(query_y);
  }
  vec_y.resize(slot_count);

  /*
 Printing the matrix is a bit of a pain.
 */
 auto print_matrix = [row_size](auto &matrix)
 {
     cout << endl;
     size_t print_size = 5;

     cout << "    [";
     for (size_t i = 0; i < print_size; i++)
     {
         cout << setw(3) << matrix[i] << ",";
     }
     cout << setw(3) << " ...,";
     for (size_t i = row_size - print_size; i < row_size; i++)
     {
         cout << setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
     }
     cout << "    [";
     for (size_t i = row_size; i < row_size + print_size; i++)
     {
       //cout<<i<<endl<<row_size<<endl<<print_size<<endl;
         cout << setw(3) << matrix[i] << ",";
     }
     cout << setw(3) << " ...,";
     for (size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
     {
         cout << setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
     }
     cout << endl;
 };

  //encoder the vector to a coefficient
  Plaintext plaintext_x;
  batch_encoder.encode(vec_x, plaintext_x);
  print_matrix(vec_x);

  Plaintext plaintext_y;
  batch_encoder.encode(vec_y, plaintext_y);
  print_matrix(vec_y);

  cout << "Encrypting: ";
  Ciphertext ciphertext_x;
  encryptor.encrypt(plaintext_x, ciphertext_x);



  // ofstream test;
  // test.open("test", ios::binary);
  // ciphertext_x.save(test);
  // test.close();
  // cout << "OK" << endl;



  Ciphertext ciphertext_y;
  encryptor.encrypt(plaintext_y, ciphertext_y);
  cout << "Done" << endl;

  //save in a file
  cout << "Save in a file." << endl;
  ofstream queryFile;
  queryFile.open("query", ios::binary);
  ciphertext_x.save(queryFile);
  ciphertext_y.save(queryFile);
  queryFile.close();
  cout << "End" << endl;

  auto endWhole=chrono::high_resolution_clock::now();
  chrono::duration<double> diffWhole = endWhole-startWhole;
  cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;

  return 0;
}
