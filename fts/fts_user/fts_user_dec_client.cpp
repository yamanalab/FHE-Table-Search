#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_user/fts_user_dec_client.hpp>

namespace fts_user
{

struct DecClient::Impl
{
public:
    Impl(const char* host, const char* port)
        : host_(host),
          port_(port),
          client_()
    {
    }

    ~Impl(void)
    {
        disconnect();
    }

    void connect(const uint32_t retry_interval_usec,
                 const uint32_t timeout_sec)
    {
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
    }

    void disconnect(void)
    {
        client_.close();
    }

    int32_t new_keys(seal::SecretKey& seckey)
    {
        stdsc::Buffer buffer;
        client_.recv_data_blocking(fts_share::kControlCodeDownloadNewKeys, buffer);

        stdsc::BufferStream buffstream(buffer);
        std::iostream stream(&buffstream);

        fts_share::PlainData<int32_t> plaindata;
        plaindata.load_from_stream(stream);
        
        seckey.unsafe_load(stream);
        
        return plaindata.data();
    }

    bool delete_keys(const int32_t key_id) const
    {
        return true;
    }

    template <class T>
    void get_key(const int32_t key_id, const fts_share::ControlCode_t code, T& key)
    {
        stdsc::Buffer sbuffer(sizeof(key_id)), rbuffer;
        *(int32_t*)sbuffer.data() = key_id;
        client_.send_recv_data_blocking(code, sbuffer, rbuffer);

        stdsc::BufferStream buffstream(rbuffer);
        std::iostream stream(&buffstream);
        key.unsafe_load(stream);
    }
    
    void get_param(const int32_t key_id, seal::EncryptionParameters& param)
    {
        stdsc::Buffer sbuffer(sizeof(key_id)), rbuffer;
        *(int32_t*)sbuffer.data() = key_id;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadParam, sbuffer, rbuffer);
    
        stdsc::BufferStream buffstream(rbuffer);
        std::iostream stream(&buffstream);
        param = seal::EncryptionParameters::Load(stream);
    }

private:
    const char* host_;
    const char* port_;
    stdsc::Client client_;
};

DecClient::DecClient(const char* host, const char* port)
    : pimpl_(new Impl(host, port))
{
}

void DecClient::connect(const uint32_t retry_interval_usec,
                       const uint32_t timeout_sec)
{
    STDSC_LOG_INFO("Connect to decryptor.");
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void DecClient::disconnect(void)
{
    STDSC_LOG_INFO("Disconnect from decryptor.");
    pimpl_->disconnect();
}

int32_t DecClient::new_keys(seal::SecretKey& seckey)
{
    STDSC_LOG_INFO("New keys: sending new key request to decryptor.");
    auto key_id = pimpl_->new_keys(seckey);
    STDSC_LOG_INFO("New keys: received key ID (#%d)", key_id);
    return key_id;
}

bool DecClient::delete_keys(const int32_t key_id) const
{
    STDSC_LOG_INFO("Delete keys: sending delete key request to decryptor.");
    bool res = pimpl_->delete_keys(key_id);
    STDSC_LOG_INFO("Delete keys: received result: %s", res? "success" : "failed");
    return res;
}

void DecClient::get_pubkey(const int32_t key_id, seal::PublicKey& pubkey)
{
    STDSC_LOG_INFO("Get public key: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadPubKey, pubkey);
    STDSC_LOG_INFO("Get public key: received key of #%d", key_id);
}
    
void DecClient::get_galoiskey(const int32_t key_id, seal::GaloisKeys& galoiskey)
{
    STDSC_LOG_INFO("Get galois keys: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadGaloisKey, galoiskey);
    STDSC_LOG_INFO("Get galois keys: received key of #%d", key_id);
}
    
void DecClient::get_relinkey(const int32_t key_id, seal::RelinKeys& relinkey)
{
    STDSC_LOG_INFO("Get relin keys: sending request of #%d to decryptor.", key_id);
    pimpl_->get_key(key_id, fts_share::kControlCodeUpDownloadRelinKey, relinkey);
    STDSC_LOG_INFO("Get relin keys: received key of #%d", key_id);
}

void DecClient::get_param(const int32_t key_id, seal::EncryptionParameters& param)
{
    STDSC_LOG_INFO("Get encryption parameters: sending request of #%d to decryptor.", key_id);
    pimpl_->get_param(key_id, param);
    STDSC_LOG_INFO("Get encryption parameters: received key of #%d", key_id);
}

} /* namespace fts_user */
