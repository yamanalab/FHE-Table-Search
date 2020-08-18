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

#include <fts_share/fts_user2decparam.hpp>

namespace fts_share
{

std::ostream& operator<<(std::ostream& os, const User2DecParam& param)
{
    os << param.poly_mod_degree  << std::endl;
    os << param.coef_mod_192     << std::endl;
    os << param.plain_mod        << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, User2DecParam& param)
{
    is >> param.poly_mod_degree;
    is >> param.coef_mod_192;
    is >> param.plain_mod;
    return is;
}
    
} /* namespace fts_share */
