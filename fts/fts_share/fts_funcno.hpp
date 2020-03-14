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

#ifndef FTS_FUNCNO_HPP
#define FTS_FUNCNO_HPP

namespace fts_share
{

/**
 * @brief Enumeration for function No.
 */
enum FuncNo_t : int32_t
{
    kFuncNil    = 0,
    kFuncOne    = 1,
    kFuncTwo    = 2,
};
    
} /* namespace fts_share */

#endif /* FTS_FUNCNO_HPP */
