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

#ifndef FTS_CS_CALCTHREAD_HPP
#define FTS_CS_CALCTHREAD_HPP

#include <memory>
#include <cstdbool>
#include <vector>
#include <stdsc/stdsc_thread.hpp>

namespace fts_cs
{
    
class CalcThreadParam;
class QueryQueue;
class ResultQueue;

/**
 * @brief Calculation thread
 */
class CalcThread : public stdsc::Thread<CalcThreadParam>
{
    using super = Thread<CalcThreadParam>;
public:
    /**
     * Constructor
     * @param[in] in_queue query queue
     * @param[out] out_queue result queue
     * @param[in] LUTin_one  input LUT for one input
     * @param[in] LUTin_two  input LUT for two input
     * @param[in] LUTout_two output LUT for two input
     * @param[in] possible_input_num_one num of possible inputs for one input
     * @param[in] possible_input_num_two num of possible inputs for two input
     * @param[in] possible_combination_num_two num of combination for one input
     * @param[in] dec_host hostname of decryptor
     * @param[in] dec_port port number of decryptor
     */
    CalcThread(QueryQueue& in_queue,
               ResultQueue& out_queue,
               std::vector<std::vector<int64_t>>& LUTin_one,
               std::vector<std::vector<int64_t>>& LUTin_two,
               std::vector<int64_t>& LUTout_two,
               const int64_t possible_input_num_one,
               const int64_t possible_input_num_two,
               const int64_t possible_combination_num_two,
               const std::string& dec_host,
               const std::string& dec_port);
    virtual ~CalcThread(void) = default;

    /**
     * Start thread
     */
    void start();
    
    /**
     * Stop thread
     */
    void stop();
    
private:
    virtual void exec(CalcThreadParam& args,
                      std::shared_ptr<stdsc::ThreadException> te) const override;

    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief This class is used to hold the calcration parameters for CalcThread.
 */
struct CalcThreadParam
{
    uint32_t retry_interval_msec = DefaultRetryIntervalMsec;
    bool force_finish = false;

    static constexpr uint32_t DefaultRetryIntervalMsec = 100;
};

} /* namespace fts_cs */

#endif /* FTS_CS_CALCTHREAD_HPP */
