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

#ifndef FTS_USER2DECPARAM_HPP
#define FTS_USER2DECPARAM_HPP

#include <iostream>

namespace fts_share
{

/**
 * @brief This class is used to hold the parameters to compute on encryptor.
 */
struct User2DecParam
{
    std::size_t poly_mod_degree = DefaultPolyModDegree;
    std::size_t coef_mod_192    = DefaultCoefMod192;
    std::size_t plain_mod       = DefaultPlainMod;

    static constexpr std::size_t DefaultPolyModDegree = 8192;
    static constexpr std::size_t DefaultCoefMod192    = 8192;
    static constexpr std::size_t DefaultPlainMod      = 786433;
};

std::ostream& operator<<(std::ostream& os, const User2DecParam& param);
std::istream& operator>>(std::istream& is, User2DecParam& param);

} /* namespace fts_share */

#endif /* FTS_USER2DECPARAM_HPP */
