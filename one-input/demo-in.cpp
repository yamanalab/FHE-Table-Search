//201811@Richelle
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
  int64_t k = ceil(TABLE_SIZE_N/row_size);

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator(seed);

  //string s1(argv[1]);
  //make query
  cout<<"Get query!";
  // string LUT_query_temp;
  // cin >> LUT_query_temp;
  // int64_t LUT_query=std::atoi(LUT_query_temp);
  int64_t LUT_query=2;

  //encrypt the LUT query
  cout << "Encrypt and save your query..." << flush;
  vector<int64_t> query;
  for(int i=0 ; i<row_size ; i++){
    query.push_back(LUT_query);
  }
  query.resize(slot_count);

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
  Plaintext plaintext_query;
  batch_encoder.encode(query, plaintext_query);
  //encoder the vector to a coefficient
  //cout << "Plaintext polynomial: " << plaintext_query.to_string() << endl;
  print_matrix(query);

  Ciphertext ciphertext_query;
  cout << "Encrypting: ";
  encryptor.encrypt(plaintext_query, ciphertext_query);
  cout << "Done" << endl;

  //save in a file
  cout << "Save in a file." << endl;
  ofstream queryFile;
  queryFile.open("query", ios::binary);
  ciphertext_query.save(queryFile);
  queryFile.close();
  cout << "End" << endl;

  return 0;
}
