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

#include <fts_cs/fts_cs_result.hpp>
#include <seal/seal.h>

namespace fts_cs
{

// Result
Result::Result(const int32_t query_id, const bool status, const seal::Ciphertext& ctxt)
    : query_id_(query_id),
      status_(status),
      ctxt_(ctxt)
{
    created_time_ = std::chrono::system_clock::now();
}

double Result::elapsed_time() const
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - created_time_).count();
}
    
} /* namespace fts_cs */
