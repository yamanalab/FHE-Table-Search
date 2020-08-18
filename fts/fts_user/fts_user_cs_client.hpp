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

#ifndef FTS_CLIENT_CS_CLIENT_HPP
#define FTS_CLIENT_CS_CLIENT_HPP

#include <memory>
#include <fts_share/fts_define.hpp>
#include <fts_user/fts_user_result_cbfunc.hpp>

namespace fts_share
{
    class EncData;
}

namespace fts_user
{

/**
 * @brief Provides client for Computation Server.
 */
class CSClient
{
public:
    /**
     * Constructor
     * @param[in] host hostname
     * @param[in] port port number
     * @param[in] enc_params parameters for seal
     */
    CSClient(const char* host, const char* port,
             const seal::EncryptionParameters& enc_params);
    virtual ~CSClient(void) = default;

    /**
     * Connect
     * @param[in] retry_interval_usec retry interval (usec)
     * @param[in] timeout_sec timeout (sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * Disconnect
     */
    void disconnect();
    
    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] func_no function number
     * @param[in] enc_input encrypted input values (1 or 2)
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs) const;

    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] func_no function number
     * @param[in] enc_input encrypted input values (1 or 2)
     * @param[in] cbfunc callback function
     * @param[in] cbfunc_args arguments for callback function
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs,
                       cbfunc_t cbfunc,
                       void* cbfunc_args) const;
    
    /**
     * Receive results
     * @param[in] query_id    query ID
     * @param[out] status     calcuration status
     * @param[out] enc_result encrypted result
     */
    void recv_results(const int32_t query_id, bool& status, fts_share::EncData& enc_result) const;

    /**
     * Set callback functions
     * @param[in] query_id queryID
     * @param[in] func callback function
     * @param[in] args arguments for callback function
     */
    void set_callback(const int32_t query_id, cbfunc_t funvc, void* args) const;

    /**
     * Wait for finish of query
     * @param[in] query_id query ID
     */
    void wait(const int32_t query_id) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_user */

#endif /* FTS_USER_CS_CLIENT_HPP */
