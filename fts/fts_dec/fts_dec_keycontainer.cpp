#include <fstream>
#include <unordered_map>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_user2decparam.hpp>
#include <fts_dec/fts_dec_keycontainer.hpp>

#include <seal/seal.h>


#define CHECK_KIND(k) do {                                               \
        if (!((k) < kNumOfKind)) {                                       \
            std::ostringstream oss;                                      \
            oss << "Err: Invalid securekey kind. (kind: " << (k) << ")"; \
            STDSC_THROW_INVPARAM(oss.str().c_str());                     \
        }                                                                \
    } while(0)


namespace fts_dec
{

void print_parameters(std::shared_ptr<seal::SEALContext> context)
{
    // Verify parameters
    STDSC_THROW_INVPARAM_IF_CHECK(context, "context is not set");
    auto &context_data = *context->context_data();

    //Which scheme are we using?
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        STDSC_THROW_INVARIANT("unsupported scheme");
    }

    std::cout << "/ Encryption parameters:" << std::endl;
    std::cout << "| scheme: " << scheme_name << std::endl;
    std::cout << "| poly_modulus_degree: " <<
        context_data.parms().poly_modulus_degree() << std::endl;

    //Print the size of the true (product) coefficient modulus.
    std::cout << "| coeff_modulus size: " << context_data.
        total_coeff_modulus_bit_count() << " bits" << std::endl;

    //For the BFV scheme print the plain_modulus parameter.
    if (context_data.parms().scheme() == seal::scheme_type::BFV)  {
        std::cout << "| plain_modulus: " << context_data.
            parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\ noise_standard_deviation: " << context_data.
        parms().noise_standard_deviation() << std::endl;
    std::cout << std::endl;
}
    
struct KeyContainer::Impl
{
    struct KeyFilenames
    {    
        KeyFilenames(const int32_t id)
        {
            filenames_.emplace(kKindPubKey,    std::string("pubkey_")    + std::to_string(id));
            filenames_.emplace(kKindSecKey,    std::string("seckey_")    + std::to_string(id));
            filenames_.emplace(kKindGaloisKey, std::string("galoiskey_") + std::to_string(id));
            filenames_.emplace(kKindRelinKey,  std::string("relinkey_")  + std::to_string(id));
            filenames_.emplace(kKindParam,     std::string("param_")     + std::to_string(id));
        }

        std::string filename(const KeyKind_t kind) const
        {
            CHECK_KIND(kind);
            return filenames_.at(kind);
        }
        
        std::unordered_map<KeyKind_t, std::string> filenames_;
    };
    
    Impl()
    {}

    int32_t new_keys(const fts_share::User2DecParam& param)
    {
        int32_t key_id = fts_share::utility::gen_uuid();
        map_.emplace(key_id, KeyFilenames(key_id));
        generate_keyfiles(param.poly_mod_degree, param.coef_mod_192, param.plain_mod, map_.at(key_id));
        return key_id;
    }
    
    void delete_keys(const int32_t key_id)
    {
        remove_keyfiles(map_.at(key_id));
        map_.erase(key_id);
    }

    template <class T>
    void get(const int32_t key_id, const KeyKind_t kind, T& data) const
    {
        const auto& filename = map_.at(key_id).filename(kind);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(oss.str());
        }
        std::ifstream ifs(filename);
        data.unsafe_load(ifs);

        ifs.close();
    }

    void get_param(const int32_t key_id, seal::EncryptionParameters& param) const
    {
        const auto& filename = map_.at(key_id).filename(KeyKind_t::kKindParam);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(oss.str());
        }
        std::ifstream ifs(filename);
        param = seal::EncryptionParameters::Load(ifs);
        ifs.close();
    }

    size_t data_size(const int32_t key_id, const KeyKind_t kind) const
    {
        const auto& filename = map_.at(key_id).filename(kind);
        if (!fts_share::utility::file_exist(filename)) {
            std::ostringstream oss;
            oss << "File is not found. (" << filename << ")";
            STDSC_THROW_FILE(oss.str());
        }
        return fts_share::utility::file_size(filename);
    }
    
private:
    
    void generate_keyfiles(const std::size_t poly_mod_degree,
                           const std::size_t coef_mod_192,
                           const std::size_t plain_mod,
                           const KeyFilenames& filenames)
    {
        STDSC_LOG_INFO("Generating keys");
        seal::EncryptionParameters parms(seal::scheme_type::BFV);
        parms.set_poly_modulus_degree(poly_mod_degree);
        parms.set_coeff_modulus(seal::DefaultParams::coeff_modulus_192(coef_mod_192));
        parms.set_plain_modulus(plain_mod);

        auto context = seal::SEALContext::Create(parms);
        print_parameters(context);

        seal::KeyGenerator keygen(context);
        seal::PublicKey public_key = keygen.public_key();
        seal::SecretKey secret_key = keygen.secret_key();
        auto gal_keys = keygen.galois_keys(16);
        auto relin_keys16 = keygen.relin_keys(16);
        seal::BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "Slot nums = " << slot_count << std::endl;

        std::cout << "Save public key and secret key..." << std::flush;
        std::ofstream pkFile(filenames.filename(KeyKind_t::kKindPubKey), std::ios::binary);
        public_key.save(pkFile);
        pkFile.close();

        std::ofstream skFile(filenames.filename(KeyKind_t::kKindSecKey), std::ios::binary);
        secret_key.save(skFile);
        skFile.close();

        std::ofstream parmsFile(filenames.filename(KeyKind_t::kKindParam), std::ios::binary);
        seal::EncryptionParameters::Save(parms, parmsFile);
        parmsFile.close();

        std::ofstream galFile(filenames.filename(KeyKind_t::kKindGaloisKey), std::ios::binary);
        gal_keys.save(galFile);
        galFile.close();

        std::ofstream relinFile(filenames.filename(KeyKind_t::kKindRelinKey), std::ios::binary);
        relin_keys16.save(relinFile);
        relinFile.close();

        std::cout << "End" << std::endl;
    }

    void remove_keyfiles(const KeyFilenames& filenames)
    {
        int32_t bgn = static_cast<int32_t>(KeyKind_t::kKindPubKey);
        int32_t end = static_cast<int32_t>(KeyKind_t::kKindParam);

        for (auto i=bgn; i<=end; ++i) {
            const auto key = static_cast<KeyKind_t>(i);
            const auto& filename = filenames.filename(key);
            auto ret = fts_share::utility::remove_file(filename);
            if (!ret) {
                std::ostringstream oss;
                oss << "Failed to remove file. (" << filename << ")";
                STDSC_THROW_FILE(oss.str());
            }
        }
    }
    
private:
    std::unordered_map<int32_t, KeyFilenames> map_;
};

KeyContainer::KeyContainer()
    : pimpl_(new Impl())
{}

int32_t KeyContainer::new_keys(const fts_share::User2DecParam& param)
{
    auto key_id = pimpl_->new_keys(param);
    STDSC_LOG_INFO("Generate new keys. (key ID: %d)", key_id);
    return key_id;
}

void KeyContainer::delete_keys(const int32_t key_id)
{
    pimpl_->delete_keys(key_id);
    STDSC_LOG_INFO("Deleted key #d.", key_id);
}

template <class T>
void KeyContainer::get(const int32_t key_id, const KeyKind_t kind, T& data) const
{
    pimpl_->get<T>(key_id, kind, data);
}

#define DEF_GET_WITH_TYPE(type, name)                                     \
    template <>                                                           \
    void KeyContainer::get(const int32_t key_id,                          \
                           const KeyKind_t kind, type& data) const {      \
        STDSC_LOG_INFO("Get keys. (key ID: %d, kind: %s)", key_id, name); \
        pimpl_->get(key_id, kind, data);                                  \
    }

    DEF_GET_WITH_TYPE(seal::PublicKey,  "public key");
    DEF_GET_WITH_TYPE(seal::SecretKey,  "secret key");
    DEF_GET_WITH_TYPE(seal::GaloisKeys, "galois keys")
    DEF_GET_WITH_TYPE(seal::RelinKeys,  "relin keys");

#undef DEF_GET_WITH_TYPE

size_t KeyContainer::data_size(const int32_t key_id, const KeyKind_t kind) const
{
    return pimpl_->data_size(key_id, kind);
}

void KeyContainer::get_param(const int32_t key_id, seal::EncryptionParameters& param) const
{
    STDSC_LOG_INFO("Get encryption parameters. (key ID: %d)", key_id);
    pimpl_->get_param(key_id, param);
}

} /* namespace fts_dec */
