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

#ifndef FTS_CS_CALCMANAGER_HPP
#define FTS_CS_CALCMANAGER_HPP

#include <memory>
#include <cstdbool>
#include <string>

namespace fts_cs
{

class Query;
class Result;

class CalcManager
{
public:
    /**
     * Constructor
     * @param[in] LUT_dir                LUT directory
     * @param[in] max_concurrent_queries max number of concurrent queries
     * @param[in] max_results max        result number to hold
     * @param[in] result_lifetime_sec    lifetime to hold (sec)
     */
    CalcManager(const std::string& LUT_dir,
                const uint32_t max_concurrent_queries,
                const uint32_t max_results,
                const uint32_t result_lifetime_sec);
    virtual ~CalcManager() = default;

    /**
     * Start calculation threads
     * @param[in] thread_num number of threads
     * @param[in] dec_host hostname of decryptor
     * @param[in] dec_port port number of decryptor
     */
    void start_threads(const uint32_t thread_num,
                       const std::string& dec_host,
                       const std::string& dec_port);

    /**
     * Stop calculation threads
     */
    void stop_threads();

    /**
     * Set queries
     * @param[in] query query
     * @return query ID
     */
    int32_t push_query(const Query& query);

    /**
     * Get results of query
     * @paran[in] query_id query ID
     * @param[out] result result
     * @param[in] retry_interval_usec retry interval (usec)
     */
    void pop_result(const int32_t query_id, Result& result,
                    const uint32_t retry_interval_msec=100) const;

    /**
     * Delete results if number of results grater than max number and expired lifetime
     */
    void cleanup_results();

private:
    class Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_cs */

#endif /* FTS_CS_CALCMANAGER_HPP */
