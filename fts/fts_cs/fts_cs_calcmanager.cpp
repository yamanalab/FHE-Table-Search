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

#include <vector>
#include <unistd.h>
#include <fstream>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_define.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_lut.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>


namespace fts_cs
{
    
    struct CalcManager::Impl
    {
        Impl(const std::string& LUT_dir,
             const uint32_t max_concurrent_queries,
             const uint32_t max_results,
             const uint32_t result_lifetime_sec)
            : max_concurrent_queries_(max_concurrent_queries),
              max_results_(max_results),
              result_lifetime_sec_(result_lifetime_sec)
        {
            LUTLFunc LUTlfunc;
            LUTQFunc LUTqfunc;
            auto files = fts_share::utility::get_filelist(LUT_dir, FTS_LUTFILE_EXT);
            for (const auto& f : files) {
                auto func = fts_cs_lut_get_funcnumber(f);
                if (func == kLUTFuncLinear) {
                    LUTlfunc.load_from_file(f);
                    convertLUT_to_vecfmt_one(LUTlfunc, LUTin_one_, possible_input_num_one_);
                } else if (func == kLUTFuncQuadratic) {
                    LUTqfunc.load_from_file(f);
                    convertLUT_to_vecfmt_two(LUTqfunc, LUTin_two_, LUTout_two_,
                                             possible_input_num_two_, possible_combination_num_two_);
                } else {
                    STDSC_THROW_FILE("The LUT file has an invalid format.");
                }
            }
        }

        void convertLUT_to_vecfmt_one(LUTLFunc& lut,
                                      std::vector<std::vector<int64_t>>& lutvec_io,
                                      int64_t& possible_input_num) const
        {
            lutvec_io.clear();
            lutvec_io.resize(2); // [0]: input cols (x), [1]: output cols (y)
            for (const auto& pair : lut) {
                auto& key = pair.first;
                auto& val = pair.second;
                lutvec_io[0].push_back(stol(key));
                lutvec_io[1].push_back(val);
            }
            lutvec_io[0].resize(FTS_LUT_POSSIBLE_INPUT_NUM_ONE, 100);
            lutvec_io[1].resize(FTS_LUT_POSSIBLE_INPUT_NUM_ONE, 100);            
            possible_input_num = FTS_LUT_POSSIBLE_INPUT_NUM_ONE;
        }

        void convertLUT_to_vecfmt_two(LUTQFunc& lut,
                                      std::vector<std::vector<int64_t>>& lutvec_i,
                                      std::vector<int64_t>& lutvec_o,
                                      int64_t& possible_input_num,
                                      int64_t& possible_combination_num) const
        {
            lutvec_i.clear();
            lutvec_o.clear();
            lutvec_i.resize(2); // [0]: input cols (x0), [1]: input cols (x1)
            for (const auto& pair : lut) {
                auto& x0x1 = lut.decode_key(pair.first);
                lutvec_i[0].push_back(x0x1.first);  // x0
                lutvec_i[1].push_back(x0x1.second); // x1
            }

            std::sort(lutvec_i[0].begin(), lutvec_i[0].end());
            std::sort(lutvec_i[1].begin(), lutvec_i[1].end());
            lutvec_i[0].erase(std::unique(lutvec_i[0].begin(), lutvec_i[0].end()), lutvec_i[0].end());
            lutvec_i[1].erase(std::unique(lutvec_i[1].begin(), lutvec_i[1].end()), lutvec_i[1].end());

            size_t x0sz = lutvec_i[0].size();
            size_t x1sz = lutvec_i[1].size();
            
            STDSC_THROW_INVPARAM_IF_CHECK(x0sz < FTS_LUT_POSSIBLE_INPUT_NUM_TWO, "size of x0 in LUT of two input size too large");
            STDSC_THROW_INVPARAM_IF_CHECK(x1sz < FTS_LUT_POSSIBLE_INPUT_NUM_TWO, "size of x1 in LUT of two input size too large");

            lutvec_i[0].resize(FTS_LUT_POSSIBLE_INPUT_NUM_TWO, 100);
            lutvec_i[1].resize(FTS_LUT_POSSIBLE_INPUT_NUM_TWO, 100);

            for (size_t i=0; i<lutvec_i[0].size(); ++i) {
                
                const auto& x0 = lutvec_i[0][i];
                for (size_t j=0; j<lutvec_i[1].size(); ++j) {
                    
                    int64_t val = 1000;
                    if (i < x0sz && j < x1sz) {
                        try {
                            const auto& x1 = lutvec_i[1][j];
                            val = lut.get(x0, x1);
                        }catch (stdsc::InvParamException& ex) {
                            // nothing to do
                        }
                    }
                    
                    lutvec_o.push_back(val);
                }
            }
            
            possible_input_num = FTS_LUT_POSSIBLE_INPUT_NUM_TWO;
            possible_combination_num = possible_input_num * possible_input_num;
        }

        const uint32_t max_concurrent_queries_;
        const uint32_t max_results_;
        const uint32_t result_lifetime_sec_;
        QueryQueue qque_;
        ResultQueue rque_;
        std::vector<std::vector<int64_t>> LUTin_one_;
        std::vector<std::vector<int64_t>> LUTin_two_;
        std::vector<int64_t> LUTout_two_;
        int64_t possible_input_num_one_;
        int64_t possible_input_num_two_;
        int64_t possible_combination_num_two_;
        std::vector<std::shared_ptr<CalcThread>> threads_;
    };

    CalcManager::CalcManager(const std::string& LUT_dir,
                             const uint32_t max_concurrent_queries,
                             const uint32_t max_results,
                             const uint32_t result_lifetime_sec)
        :pimpl_(new Impl(LUT_dir,
                         max_concurrent_queries,
                         max_results,
                         result_lifetime_sec))
    {}

    void CalcManager::start_threads(const uint32_t thread_num,
                                    const std::string& dec_host,
                                    const std::string& dec_port)
    {
        STDSC_LOG_INFO("Start calculation threads. (n:%d)", thread_num);
        pimpl_->threads_.clear();
        for (size_t i=0; i<thread_num; ++i) {
            pimpl_->threads_.emplace_back(
                std::make_shared<CalcThread>(pimpl_->qque_,
                                             pimpl_->rque_,
                                             pimpl_->LUTin_one_,
                                             pimpl_->LUTin_two_,
                                             pimpl_->LUTout_two_,
                                             pimpl_->possible_input_num_one_,
                                             pimpl_->possible_input_num_two_,
                                             pimpl_->possible_combination_num_two_,
                                             dec_host,
                                             dec_port));
        }

        for (const auto& thread : pimpl_->threads_) {
            thread->start();
        }
    }
    
    void CalcManager::stop_threads()
    {
        STDSC_LOG_INFO("Stop calculation threads.");
    }
    
    int32_t CalcManager::push_query(const Query& query)
    {
        STDSC_LOG_INFO("Set queries.");
        int32_t query_id = -1;
        
        if (pimpl_->qque_.size() < pimpl_->max_concurrent_queries_ &&
            pimpl_->rque_.size() < pimpl_->max_results_) {
            try {
                query_id = pimpl_->qque_.push(query);
            } catch (stdsc::AbstractException& ex) {
                STDSC_LOG_WARN(ex.what());
            }
        }
            
        return query_id;
    }

    void CalcManager::pop_result(const int32_t query_id, Result& result,
                                 const uint32_t retry_interval_msec) const
    {
        STDSC_LOG_INFO("Getting results of query. (retry_interval_msec: %u ms)", retry_interval_msec);
        while (!pimpl_->rque_.pop(query_id, result)) {
            usleep(retry_interval_msec * 1000);
        }
    }

    void CalcManager::cleanup_results()
    {
        if (pimpl_->rque_.size() >= pimpl_->max_results_) {
            std::vector<int32_t> query_ids;
            for (const auto& pair : pimpl_->rque_) {
                const auto& query_id = pair.first;
                const auto& result   = pair.second;
                if (result.elapsed_time() >= pimpl_->result_lifetime_sec_) {
                    STDSC_LOG_INFO("Deleted the results of query%d because it has expired.", query_id);
                    query_ids.push_back(query_id);
                }
            }
            Result tmp;
            for (const auto& id : query_ids) {
                pimpl_->rque_.pop(id, tmp);
            }
        }
    }

} /* namespace fts_cs */
