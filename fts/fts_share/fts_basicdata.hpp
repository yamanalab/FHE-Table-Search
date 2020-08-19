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

#ifndef FTS_BASICDATA_HPP
#define FTS_BASICDATA_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_utility.hpp>

namespace fts_share
{

/**
 * @brief This class is template of data.
 */
template <class T>
struct BasicData
{
    BasicData(void) = default;
    virtual ~BasicData(void) = default;

    virtual void push(const T& data)
    {
        vec_.push_back(data);
    }

    virtual void clear(void)
    {
        vec_.clear();
    }

    virtual void save_to_stream(std::ostream& os) const
    {
        STDSC_LOG_WARN("%s is not implemented.", __FUNCTION__);
    }
    
    virtual void load_from_stream(std::istream& is)
    {
        STDSC_LOG_WARN("%s is not implemented.", __FUNCTION__);
    }

    virtual void save_to_file(const std::string& filepath) const
    {
        std::ofstream ofs(filepath, std::ios::binary);
        save_to_stream(ofs);
        ofs.close();
    }
    
    virtual void load_from_file(const std::string& filepath)
    {
        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }
        std::ifstream ifs(filepath, std::ios::binary);
        load_from_stream(ifs);
        ifs.close();
    }
    
    virtual const T& data(void) const
    {
        STDSC_THROW_FAILURE_IF_CHECK(vec_.size() > 0, "Data is empty.");
        return vec_[0];
    }
    
    virtual T& data(void)
    {
        STDSC_THROW_FAILURE_IF_CHECK(vec_.size() > 0, "Data is empty.");
        return vec_[0];
    }
    
    virtual const std::vector<T>& vdata(void) const
    {
        STDSC_THROW_FAILURE_IF_CHECK(vec_.size() > 0, "Data is empty.");
        return vec_;
    }
    
    virtual std::vector<T>& vdata(void)
    {
        STDSC_THROW_FAILURE_IF_CHECK(vec_.size() > 0, "Data is empty.");
        return vec_;
    }        

    virtual size_t stream_size(void) const
    {
        std::ostringstream oss;
        save_to_stream(oss);
        return oss.str().size();
    }

protected:
    std::vector<T> vec_;
};

} /* namespace fts_share */

#endif /* FTS_BASICDATA_HPP */
