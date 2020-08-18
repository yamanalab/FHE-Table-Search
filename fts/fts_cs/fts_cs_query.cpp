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

#include <fts_share/fts_utility.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <seal/seal.h>

namespace fts_cs
{
Query::Query(const int32_t key_id, const fts_share::FuncNo_t func_no,
             const std::vector<seal::Ciphertext>& ctxts)
    : key_id_(key_id),
      func_no_(func_no)
{
    ctxts_.resize(ctxts.size());
    std::copy(ctxts.begin(), ctxts.end(), ctxts_.begin());
}

int32_t QueryQueue::push(const Query& data)
{
    auto id = fts_share::utility::gen_uuid();
    super::push(id, data);
    return id;
}

} /* namespace fts_cs */
