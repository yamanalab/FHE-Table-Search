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

#ifndef FTS_SEAL_UTILITY_HPP
#define FTS_SEAL_UTILITY_HPP

#include <string>
#include <vector>

namespace seal
{
    class EncryptionParameters;
}

namespace fts_share
{

namespace seal_utility
{
    template <class T>
    void write_to_file(const std::string& filepath, const T& data);

    template <class T>
    void write_to_file(const std::string& filepath, const std::vector<T>& vdata);
    
    template <>
    void write_to_file<seal::EncryptionParameters>(const std::string& filepath,
                                                   const seal::EncryptionParameters& params);

    template <class T>
    size_t stream_size(const T& data);

    template <>
    size_t stream_size<seal::EncryptionParameters>(const seal::EncryptionParameters& params);

} /* namespace seal_utility */

} /* namespace fts_share */

#endif /* FTS_SEAL_UTILITY_HPP */
