//201905@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"

using namespace seal;
using namespace std;

void output_plaintext(const vector<vector <int64_t>> &a){
  int64_t row=a.size();
  int64_t col=a[0].size();

    for(int i=0 ; i<row ; i++){
        for(int j=0 ; j<col ; j++)
            cout<<a[i][j]<<" ";
        cout<<endl;
	cout<<i<<endl;
    }
}

void out_vector(const vector<int64_t> &a){
  for(int j=0 ; j<a.size() ; j++){
    //cout<<a[j]<<" ";
    //if(a[j]==2) cout<<"\033[1;33mHere\033[0m";
    if(a[j]!= 0) cout<<"\033[1;33m "<<a[j]<<"\033[0m";
    //else cout<<a[j]<<" ";
  }
  cout<<endl;
}

int main(int argc, char *argv[]){
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

  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  BatchEncoder batch_encoder(context);
  size_t slot_count = batch_encoder.slot_count();
  size_t row_size = slot_count / 2;
  cout << "Plaintext matrix row size: " << row_size << endl;
  cout << "Slot nums = " << slot_count << endl;
  IntegerEncoder encoder(context);

  int64_t l = row_size;
  int64_t k = ceil(TABLE_SIZE_N/row_size);
/////////////////////////////////////////////////////////

  string s1(argv[1]);
  ifstream result;
  Ciphertext ciphertext_query0,ciphertext_query1,ciphertext_query2;
  Plaintext plaintext_query0,plaintext_query1,plaintext_query2;
  result.open(s1, ios::binary);
  ciphertext_query0.load(context, result);
  // ciphertext_query1.load(context, result);
  // ciphertext_query2.load(context, result);
  result.close();
  cout << "End" << endl;

  cout << "The result from CS"<< endl;
  decryptor.decrypt(ciphertext_query0, plaintext_query0);
  // decryptor.decrypt(ciphertext_query1, plaintext_query1);
  // decryptor.decrypt(ciphertext_query2, plaintext_query2);
///////////////////////////////////////////
// vector<Ciphertext> ct_result;
// vector<vector<int64_t> > dec_result(k);
// vector<Plaintext> poly_dec_result;
// for(int s=0 ; s<k ; s++){
//   Plaintext ex;
//   poly_dec_result.push_back(ex);
// }
//   ifstream mid_result("de.txt", ios::binary);
//   Ciphertext temp;
//   for(int w = 0; w < k ; w++) {
//     cout<<"w: "<<w<<endl;
//     temp.load(context, mid_result);
//     ct_result.push_back(temp);
//   }
//   mid_result.close();
//   cout << "OK" << endl;
//   for (int z=0 ; z<k ; z++){
//     cout<<"i: "<<z<<endl;
//     decryptor.decrypt(ct_result[z], poly_dec_result[z]);
//     batch_encoder.decode(poly_dec_result[z], dec_result[z]);
//   }
//   output_plaintext(dec_result);

 // auto print_matrix = [row_size](auto &matrix)
 // {
 //     cout << endl;
 //     size_t print_size = 5;
 //
 //     cout << "    [";
 //     for (size_t i = 0; i < print_size; i++)
 //     {
 //         cout << setw(3) << matrix[i] << ",";
 //     }
 //     cout << setw(3) << " ...,";
 //     for (size_t i = row_size - print_size; i < row_size; i++)
 //     {
 //         cout << setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
 //     }
 //     cout << "    [";
 //     for (size_t i = row_size; i < row_size + print_size; i++)
 //     {
 //       //cout<<i<<endl<<row_size<<endl<<print_size<<endl;
 //         cout << setw(3) << matrix[i] << ",";
 //     }
 //     cout << setw(3) << " ...,";
 //     for (size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
 //     {
 //         cout << setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
 //     }
 //     cout << endl;
 // };

  //cout << "Plaintext polynomial: " << plaintext_query.to_string() << endl;
  //cout << endl;

  vector<int64_t> plaintext_result0,plaintext_result1,plaintext_result2;
  batch_encoder.decode(plaintext_query0, plaintext_result0);
  // batch_encoder.decode(plaintext_query1, plaintext_result1);
  // batch_encoder.decode(plaintext_query2, plaintext_result2);

  cout<<"Noise budget in result:"
      << decryptor.invariant_noise_budget(ciphertext_query0)<<" bits"<<endl;
  //evaluator.relinearize_inplace(ciphertext_query, relin_keys16);
  //cout<<"Size after relinearize: "<<ciphertext_query.size()<<endl;
  //print_matrix(plaintext_result);
  //print_matrix(plaintext_result);
   out_vector(plaintext_result0);
  // out_vector(plaintext_result1);
  // out_vector(plaintext_result2);
  // string s2(argv[2]);
  // ofstream Final_result;
  // Final_result.open(s2, ios::binary);
  // bool has_nonzero =false;
  // for(int i=0 ; i<plaintext_result.size() ; i++){
  //   if(plaintext_result[i]!=0){
  //     cout<<"i:"<<i<<" presult:"<<plaintext_result[i]<<endl;
  //     //Final_result << fin_res ;
  //     //double res=pow(10, (double)plaintext_result[i]/(-5000));
  //     Final_result<<res;
  //     has_nonzero=true;
  //     break;
  //   }
  // }
  // if (false==has_nonzero)
  //   {Final_result<<0;
  //   cout<<"0"<<endl;}
  // Final_result.close();
  return 0;
}
