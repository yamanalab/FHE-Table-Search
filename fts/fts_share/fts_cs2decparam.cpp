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

#include <fts_share/fts_cs2decparam.hpp>

namespace fts_share
{

std::ostream& operator<<(std::ostream& os, const Cs2DecParam& param)
{
    const auto i32_func_no = static_cast<int32_t>(param.func_no);
    os << i32_func_no                        << std::endl;
    os << param.key_id                       << std::endl;
    os << param.query_id                     << std::endl;
    os << param.possible_input_num_one       << std::endl;
    os << param.possible_input_num_two       << std::endl;
    os << param.possible_combination_num_two << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, Cs2DecParam& param)
{
    int32_t i32_func_no;
    is >> i32_func_no;
    is >> param.key_id;
    is >> param.query_id;
    is >> param.possible_input_num_one;
    is >> param.possible_input_num_two;
    is >> param.possible_combination_num_two;
    param.func_no = static_cast<fts_share::FuncNo_t>(i32_func_no);
    return is;
}
    
} /* namespace fts_share */
