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

#ifndef FTS_DEC_SRV_HPP
#define FTS_DEC_SRV_HPP

#include <memory>

namespace fts_share
{
    class fts_config;
}

namespace fts_dec
{

/**
 * @brief Provides Decryptor Server.
 */
class DecServer
{
public:
    /**
     * Constructor
     * @param[in] port port number
     * @param[in] callback callback functions
     * @param[in] state state machine
     */
    DecServer(const char* port,
              stdsc::CallbackFunctionContainer& callback,
              stdsc::StateContext& state);
    ~DecServer(void) = default;

    /**
     * Start server
     */
    void start(void);
    /**
     * Stop server
     */
    void stop(void);
    /**
     * Wait for stopping
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_dec */

#endif /* FTS_DEC_SRV_HPP */
