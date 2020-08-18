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

#include <memory>
#include <string>
#include <iostream>
#include <unistd.h>
#include <share/define.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_callback_function.hpp>
#include <stdsc/stdsc_callback_function_container.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_config.hpp>
#include <fts_dec/fts_dec_srv.hpp>
#include <fts_dec/fts_dec_state.hpp>
#include <fts_dec/fts_dec_callback_param.hpp>
#include <fts_dec/fts_dec_callback_function.hpp>

struct Option
{
    std::string port = PORT_DEC_SRV;
    std::string config_filename; // set empty if file is specified
};

void init(Option& option, int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "p:c:h")) != -1)
    {
        switch (opt)
        {
            case 'p':
                option.port = optarg;
                break;
            case 'c':
                option.config_filename = optarg;
                break;
            case 'h':
            default:
                printf("Usage: %s [-p port] [-c config_filename]\n", argv[0]);
                exit(1);
        }
    }
}

void exec(Option& option)
{
    stdsc::StateContext state(std::make_shared<fts_dec::StateReady>());

    stdsc::CallbackFunctionContainer callback;
    {
        std::shared_ptr<stdsc::CallbackFunction> cb_new_keys(
            new fts_dec::CallbackFunctionNewKeyRequest());
        callback.set(fts_share::kControlCodeDownloadNewKeys, cb_new_keys);
        std::shared_ptr<stdsc::CallbackFunction> cb_pub_key(
            new fts_dec::CallbackFunctionPubKeyRequest());
        callback.set(fts_share::kControlCodeUpDownloadPubKey, cb_pub_key);
        std::shared_ptr<stdsc::CallbackFunction> cb_gal_key(
            new fts_dec::CallbackFunctionGaloisKeyRequest());
        callback.set(fts_share::kControlCodeUpDownloadGaloisKey, cb_gal_key);
        std::shared_ptr<stdsc::CallbackFunction> cb_rel_key(
            new fts_dec::CallbackFunctionRelinKeyRequest());
        callback.set(fts_share::kControlCodeUpDownloadRelinKey, cb_rel_key);
        std::shared_ptr<stdsc::CallbackFunction> cb_param(
            new fts_dec::CallbackFunctionParamRequest());
        callback.set(fts_share::kControlCodeUpDownloadParam, cb_param);
        std::shared_ptr<stdsc::CallbackFunction> cb_midresult(
            new fts_dec::CallbackFunctionCsMidResult());
        callback.set(fts_share::kControlCodeUpDownloadCsMidResult, cb_midresult);
    }
    fts_dec::CallbackParam param;
    if (fts_share::utility::file_exist(option.config_filename)) {
        fts_share::Config conf;
        conf.load_from_file(option.config_filename);
#define READ(key, val, type, vfmt) do {                               \
            if (conf.is_exist_key(#key))                              \
                val = fts_share::config_get_value<type>(conf, #key); \
            STDSC_LOG_INFO("read fhe parameter. (%s: " vfmt ")",      \
                           #key, val);                                \
        } while(0)

        READ(poly_mod_degree, param.param.poly_mod_degree, size_t, "%lu");
        READ(coef_mod_192,    param.param.coef_mod_192,    size_t, "%lu");
        READ(plain_mod,       param.param.plain_mod,       size_t, "%lu");

#undef READ
    }
            
    fts_dec::CommonCallbackParam cparam;
    callback.set_commondata(static_cast<void*>(&param), sizeof(param));
    callback.set_commondata(static_cast<void*>(&cparam), sizeof(cparam),
                            stdsc::CommonDataKind_t::kCommonDataOnAllConnection);

    std::shared_ptr<fts_dec::DecServer> dec_server
        (new fts_dec::DecServer(option.port.c_str(), callback, state));

    dec_server->start();
    
    //std::string key;
    //std::cout << "hit any key to exit server: " << std::endl;
    //std::cin >> key;
    //
    //dec_server->stop();
    dec_server->wait();
}

int main(int argc, char* argv[])
{
    STDSC_INIT_LOG();
    try
    {
        Option option;
        init(option, argc, argv);
        STDSC_LOG_INFO("Launched Decryptor on port #%s", PORT_DEC_SRV);
        exec(option);
    }
    catch (stdsc::AbstractException& e)
    {
        STDSC_LOG_ERR("Err: %s", e.what());
    }
    catch (...)
    {
        STDSC_LOG_ERR("Catch unknown exception");
    }

    return 0;
}
