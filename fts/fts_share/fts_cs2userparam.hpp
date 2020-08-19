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

#ifndef FTS_CS2USERPARAM_HPP
#define FTS_CS2USERPARAM_HPP

#include <iostream>

namespace fts_share
{

/**
 * @brief Enumeration for results of computation on Cs.
 */
enum CsCalcResult_t : int32_t
{
    kCsCalcResultNil     = -1,
    kCsCalcResultSuccess = 0,
    kCsCalcResultFailed  = 1,
};

/**
 * @brief This class is used to hold the parameters to transfer from cs to user.
 */
struct Cs2UserParam
{
    CsCalcResult_t result = kCsCalcResultNil;
};

std::ostream& operator<<(std::ostream& os, const Cs2UserParam& param);
std::istream& operator>>(std::istream& is, Cs2UserParam& param);

} /* namespace fts_share */

#endif /* FTS_CS2USERPARAM_HPP */
