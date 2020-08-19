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

#ifndef FTS_CS_RESULT_HPP
#define FTS_CS_RESULT_HPP

#include <cstdint>
#include <cstdbool>
#include <chrono>
#include <fts_share/fts_concurrent_mapqueue.hpp>
#include <seal/seal.h>

namespace fts_cs
{

/**
 * @brief This class is used to hold the result data.
 */
struct Result
{
    Result() = default;
    /**
     * Constructor
     * @param[in] query_id query ID
     * @param[in] status   calcuration status
     * @param[in] ctxt     cipher text
     */
    Result(const int32_t query_id, const bool status, const seal::Ciphertext& ctxt);
    virtual ~Result() = default;

    double elapsed_time() const;

    int32_t query_id_;
    bool status_;
    seal::Ciphertext ctxt_;
    std::chrono::system_clock::time_point created_time_;
};

/**
 * @brief This class is used to hold the queue of results.
 */
struct ResultQueue : public fts_share::ConcurrentMapQueue<int32_t, Result>
{
    using super = fts_share::ConcurrentMapQueue<int32_t, Result>;
    
    ResultQueue() = default;
    virtual ~ResultQueue() = default;
};

} /* namespace fts_cs */

#endif /* FTS_CS_RESULT_HPP */
