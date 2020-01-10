//201905@Richelle
//୧(๑•̀⌄•́๑)૭✧
#include "server.hpp"
using namespace std;
using namespace seal;

//random seed
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);
/*
Helper function: Prints the name of the example in a fancy banner.
*/
void print_example_banner(string title)
{
    if (!title.empty())
    {
        size_t title_length = title.length();
        size_t banner_length = title_length + 2 + 2 * 10;
        string banner_top(banner_length, '*');
        string banner_middle = string(10, '*') + " " + title + " " + string(10, '*');

        cout << endl
            << banner_top << endl
            << banner_middle << endl
            << banner_top << endl
            << endl;
    }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
void print_parameters(shared_ptr<SEALContext> context)
{
    // Verify parameters
    if (!context)
    {
        throw invalid_argument("context is not set");
    }
    auto &context_data = *context->context_data();

    /*
    Which scheme are we using?
    */
    string scheme_name;
    switch (context_data.parms().scheme())
    {
    case scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw invalid_argument("unsupported scheme");
    }

    cout << "/ Encryption parameters:" << endl;
    cout << "| scheme: " << scheme_name << endl;
    cout << "| poly_modulus_degree: " <<
        context_data.parms().poly_modulus_degree() << endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    cout << "| coeff_modulus size: " << context_data.
        total_coeff_modulus_bit_count() << " bits" << endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == scheme_type::BFV)
    {
        cout << "| plain_modulus: " << context_data.
            parms().plain_modulus().value() << endl;
    }

    cout << "\\ noise_standard_deviation: " << context_data.
        parms().noise_standard_deviation() << endl;
    cout << endl;
}

/*
Helper function: Prints the `parms_id' to std::ostream.
*/
ostream &operator <<(ostream &stream, parms_id_type parms_id)
{
    stream << hex << parms_id[0] << " " << parms_id[1] << " "
        << parms_id[2] << " " << parms_id[3] << dec;
    return stream;
}

/*
Helper function: Prints a vector of floating-point values.
*/
template<typename T>
void print_vector(vector<T> vec, size_t print_size = 4, int prec = 3)
{
    /*
    Save the formatting information for std::cout.
    */
    ios old_fmt(nullptr);
    old_fmt.copyfmt(cout);

    size_t slot_count = vec.size();

    cout << fixed << setprecision(prec) << endl;
    if(slot_count <= 2 * print_size)
    {
        cout << "    [";
        for (size_t i = 0; i < slot_count; i++)
        {
            cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    else
    {
        vec.resize(max(vec.size(), 2 * print_size));
        cout << "    [";
        for (size_t i = 0; i < print_size; i++)
        {
            cout << " " << vec[i] << ",";
        }
        if(vec.size() > 2 * print_size)
        {
            cout << " ...,";
        }
        for (size_t i = slot_count - print_size; i < slot_count; i++)
        {
            cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    cout << endl;

    /*
    Restore the old std::cout formatting.
    */
    cout.copyfmt(old_fmt);
}

int main(int argc, char *argv[]){
    //FHE setting, generate public key and secret key

    EncryptionParameters parms(scheme_type::BFV);
    parms.set_poly_modulus_degree(8192);//8192
    parms.set_coeff_modulus(DefaultParams::coeff_modulus_192(8192));//192
    parms.set_plain_modulus(786433);//1769473 //319489 //114689

    auto context = SEALContext::Create(parms);
    print_parameters(context);

    auto qualifiers = context->context_data()->qualifiers();
    cout << "Batching enabled: " << boolalpha << qualifiers.using_batching << endl;
    //if (qualifiers.using_batching)



    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();
    auto gal_keys = keygen.galois_keys(16);
    auto relin_keys16 = keygen.relin_keys(16);
    BatchEncoder batch_encoder(context);


    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    cout << "Plaintext matrix row size: " << row_size << endl;
    cout << "Slot nums = " << slot_count << endl;

    cout << "Save public key and secret key..." << flush;
    ofstream pkFile("PublicKey",ios::binary);
    public_key.save(pkFile);
    pkFile.close();

    ofstream skFile("SecretKey",ios::binary);
    secret_key.save(skFile);
    skFile.close();

    ofstream parmsFile("Params",ios::binary);
    EncryptionParameters::Save(parms, parmsFile);
    parmsFile.close();

    ofstream galFile("GaloisKey",ios::binary);
    gal_keys.save(galFile);
    galFile.close();

    ofstream relinFile("RelinKey",ios::binary);
    relin_keys16.save(relinFile);
    relinFile.close();

    cout << "End" << endl;
    return 0;
}
