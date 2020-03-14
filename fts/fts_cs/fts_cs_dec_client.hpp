/*
 * Copyright 2018 Yamana Laboratory, Waseda University
 * Supported by JST CREST Grant Number JPMJCR1503, Japan.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE‐2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FTS_CS_DEC_CLIENT_HPP
#define FTS_CS_DEC_CLIENT_HPP

#include <memory>
#include <fts_share/fts_define.hpp>
#include <fts_share/fts_dec2csparam.hpp>
#include <seal/seal.h>

namespace fts_share
{
    class EncData;
}

namespace fts_cs
{

/**
 * @brief Provides client for Decryptor.
 */
class DecClient
{
public:
    
    /**
     * Constructor
     * @param[in] host hostname of decryptor
     * @param[in] port port number of decryptor
     */
    DecClient(const char* host, const char* port);
    virtual ~DecClient(void) = default;

    /**
     * Connect
     * @param[in] retry_interval_usec retry interval for connect to server (usec)
     * @param[in] timeout_sec timeout for connection to server (sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * Disconnect
     */
    void disconnect();

    /**
     * Get public key from decryptor
     * @param[in]  key_id key ID
     * @param[out] pubkey public key
     */
    void get_pubkey(const int32_t key_id, seal::PublicKey& pubkey);

    /**
     * Get galois keys from decryptor
     * @param[in]  key_id key ID
     * @param[out] galiskey galois keys
     */
    void get_galoiskey(const int32_t key_id, seal::GaloisKeys& galoiskey);
    
    /**
     * Get relin keys from decryptor
     * @param[in]  key_id key ID
     * @param[out] relinkey relin keys
     */
    void get_relinkey(const int32_t key_id, seal::RelinKeys& relinkey);
    
    /**
     * Get encryption parameters from decryptor
     * @param[in]  key_id key ID
     * @param[out] params encryption parameters
     */
    void get_param(const int32_t key_id, seal::EncryptionParameters& params);

    /**
     * Get PIR queries
     * @param[in] func_no function No
     * @param[in] key_id key ID
     * @parma[in] query_id query ID
     * @param[in] possible_input_num_one num of possible inputs for one input
     * @param[in] possible_input_num_two num of possible inputs for two input
     * @param[in] possible_combination_num_two num of combination for one input
     * @param[in] enc_midresult_x intermediate results
     * @param[in] enc_midresult_y intermediate results (this param is not used when one input.)
     * @param[out] enc_PIRquery PIR queries
     * @return calcuration result on decryptor
     */
    fts_share::DecCalcResult_t
    get_PIRquery(const fts_share::FuncNo_t func_no,
                 const int32_t key_id,
                 const int32_t query_id,
                 const int64_t possible_input_num_one,
                 const int64_t possible_input_num_two,
                 const int64_t possible_combination_num_two,
                 const fts_share::EncData& enc_midresult_x,
                 const fts_share::EncData& enc_midresult_y,
                 fts_share::EncData& enc_PIRquery);
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_cs */

#endif /* FTS_CS_DEC_CLIENT_HPP */
