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

#include <fstream>
#include <fts_share/fts_seal_utility.hpp>
#include <seal/seal.h>

namespace fts_share
{

namespace seal_utility
{

    template <class T>
    void write_to_file(const std::string& filepath, const T& data)
    {
        std::ofstream ofs(filepath, std::ios::binary);
        data.save(ofs);
        ofs.close();
    }
#define TEMPLATE_INSTANTIATE(type) \
    template void write_to_file(const std::string& filepath, const type& data)

    TEMPLATE_INSTANTIATE(seal::SecretKey);
    TEMPLATE_INSTANTIATE(seal::PublicKey);
    TEMPLATE_INSTANTIATE(seal::GaloisKeys);
    TEMPLATE_INSTANTIATE(seal::RelinKeys);
    TEMPLATE_INSTANTIATE(seal::Ciphertext);
    
#undef TEMPLATE_INSTANTIATE

    template <class T>
    void write_to_file(const std::string& filepath, const std::vector<T>& vdata)
    {
        std::ofstream ofs(filepath, std::ios::binary);
        for (const auto& data : vdata) {
            data.save(ofs);
        }
        ofs.close();
    }
#define TEMPLATE_INSTANTIATE(type) \
    template void write_to_file(const std::string& filepath, const std::vector<type>& vdata)

    TEMPLATE_INSTANTIATE(seal::SecretKey);
    TEMPLATE_INSTANTIATE(seal::PublicKey);
    TEMPLATE_INSTANTIATE(seal::GaloisKeys);
    TEMPLATE_INSTANTIATE(seal::RelinKeys);
    TEMPLATE_INSTANTIATE(seal::Ciphertext);
    
#undef TEMPLATE_INSTANTIATE
    
    template <>
    void write_to_file<seal::EncryptionParameters>(const std::string& filepath,
                                                   const seal::EncryptionParameters& params)
    {
        std::ofstream ofs(filepath, std::ios::binary);
        seal::EncryptionParameters::Save(params, ofs);
        ofs.close();
    }

    template <class T>
    size_t stream_size(const T& data)
    {
        std::ostringstream oss;
        data.save(oss);
        return oss.str().size();
    }

    template <>
    size_t stream_size<seal::EncryptionParameters>(const seal::EncryptionParameters& params)
    {
        std::ostringstream oss;
        seal::EncryptionParameters::Save(params, oss);
        return oss.str().size();
    }


} /* namespace seal_utility */

} /* namespace fts_share */
