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

#ifndef FTS_USER2CSPARAM_HPP
#define FTS_USER2CSPARAM_HPP

#include <iostream>
#include <fts_share/fts_funcno.hpp>

namespace fts_share
{
    
/**
 * @brief This class is used to hold the parameters to compute on encryptor.
 */
struct User2CsParam
{
    int32_t  key_id;
    FuncNo_t func_no;
};

std::ostream& operator<<(std::ostream& os, const User2CsParam& param);
std::istream& operator>>(std::istream& is, User2CsParam& param);

} /* namespace fts_share */

#endif /* FTS_USER2CSPARAM_HPP */
