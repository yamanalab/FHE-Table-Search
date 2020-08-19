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

#ifndef FTS_CS_HPP
#define FTS_CS_HPP

#include <memory>
#include <fts_share/fts_define.hpp>

namespace fts_cs
{

/**
 * @brief Provides Computation Server.
 */
class CSServer
{
public:
    /**
     * constructor
     * @param[in] port port              number
     * @param[in] dec_host               hostname of Decryptor
     * @param[in] dec_port               port number of Decryptor
     * @param[in] LUT_dir                LUT directory
     * @param[in] callback               callback functions
     * @param[in] state                  state machine
     * @param[in] max_concurrent_queries max concurrent query number
     * @param[in] max_results            max result number
     * @param[in] result_lifetime_sec    result linefile (sec)
     */
    CSServer(const char* port,
             const char* dec_host,
             const char* dec_port,
             const std::string& LUT_dir,
             stdsc::CallbackFunctionContainer& callback,
             stdsc::StateContext& state,
             const uint32_t max_concurrent_queries = FTS_DEFAULT_MAX_CONCURRENT_QUERIES,
             const uint32_t max_results = FTS_DEFAULT_MAX_RESULTS,
             const uint32_t result_lifetime_sec = FTS_DEFAULT_MAX_RESULT_LIFETIME_SEC);
    ~CSServer(void) = default;

    /**
     * start server
     */
    void start();
    /**
     * stop server
     */
    void stop(void);
    /**
     * wait for stopping
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_cs */

#endif /* FTS_CS_SRV_HPP */
