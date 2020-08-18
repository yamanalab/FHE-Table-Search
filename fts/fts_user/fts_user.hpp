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

<<<<<<< HEAD:fts/fts_share/fts_seckey.hpp
=======
>>>>>>> b535649... m
#ifndef FTS_SECKEY_HPP
#define FTS_SECKEY_HPP
=======
#ifndef FTS_USER_HPP
#define FTS_USER_HPP
>>>>>>> ba4e96b... WIP: implementing template:fts/fts_user/fts_user.hpp

#include <memory>
<<<<<<< HEAD:fts/fts_share/fts_seckey.hpp
<<<<<<< HEAD
#include <seal/seal.h>
=======
>>>>>>> b535649... m
=======
#include <vector>
#include <fts_share/fts_define.hpp>
>>>>>>> ba4e96b... WIP: implementing template:fts/fts_user/fts_user.hpp

namespace fts_user
{
<<<<<<< HEAD:fts/fts_share/fts_seckey.hpp

<<<<<<< HEAD
=======
    
>>>>>>> ba4e96b... WIP: implementing template:fts/fts_user/fts_user.hpp
/**
 * @brief Provides encryptor.
 */
class User
{
public:
    
    User(const std::string& dec_host, const std::string& dec_port,
         const std::string& cs_host, const std::string& cs_port,
         const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
         const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    virtual ~User(void) = default;

    int32_t new_keys();
    void compute(const int32_t key_id, const int64_t val);
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_user */

<<<<<<< HEAD:fts/fts_share/fts_seckey.hpp
#endif /* FTS_SECKEY_HPP */
=======
struct SecKey
{
    SecKey(void);
    ~SecKey(void) = default;
};


} /* namespace fts_share */

#endif /* FTS_SECKEY_HPP */
>>>>>>> b535649... m
=======
#endif /* FTS_USER_HPP */
>>>>>>> ba4e96b... WIP: implementing template:fts/fts_user/fts_user.hpp
