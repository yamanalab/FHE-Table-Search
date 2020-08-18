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

#ifndef FTS_CONTEXT_HPP
#define FTS_CONTEXT_HPP

#include <iostream>
#include <string>
#include <memory>
#include <seal/seal.h>

namespace fts_share
{

/**
 * @brief This class is used to hold the context.
 */
struct Context
{
    Context(void);
    ~Context(void) = default;

    void save_to_stream(std::ostream& os) const;
    void load_from_stream(std::istream& is);

    void save_to_file(const std::string& filepath) const;
    void load_from_file(const std::string& filepath);

    const seal::SEALContext& get(void) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_share */

#endif /* FTS_CONTEXT_HPP */
