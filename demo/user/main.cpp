
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
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_utility.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_user2csparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_user/fts_user_dec_client.hpp>
#include <fts_user/fts_user_cs_client.hpp>
#include <fts_user/fts_user_result_thread.hpp>

#define ENABLE_LOCAL_DEBUG

#define PRINT_USAGE_AND_EXIT() do {                         \
        printf("Usage: %s value_x [value_y]\n", argv[0]);   \
        exit(1);                                            \
    } while (0)

struct Option
{
    int64_t input_value_x = -1;
    int64_t input_value_y = -1;
    int32_t input_num = 0;
};

struct CallbackParam
{
    seal::SecretKey* seckey = nullptr;
    seal::EncryptionParameters* params = nullptr;
};

void callback_func(const int32_t query_id,
                   const bool status,
                   const seal::Ciphertext* enc_result, void* args)
{
    STDSC_LOG_INFO("Callback function for query #%d", query_id);
    const auto* callback_param = reinterpret_cast<CallbackParam*>(args);

    if (!status) {
        STDSC_LOG_WARN("Failed to computation on cs.");
        return;
    }
    
    std::vector<int64_t> result_values;
    fts_share::EncData encdata(*callback_param->params, *enc_result);
    encdata.decrypt(*callback_param->seckey, result_values);

    std::cout << "Result of query #" << query_id << ":" << std::flush;
    for (const auto& v : result_values) {
        std::cout << " " << "\033[1;33m " << v << "\033[0m";
    }
    std::cout << std::endl;

    // for test script
    for (const auto& v : result_values) {
        std::cerr << v << std::endl;
    }
}

void init(Option& option, int argc, char* argv[])
{
    if (argc < 2) {
        PRINT_USAGE_AND_EXIT();
    } else {
        if (stdsc::utility::isdigit(argv[1])) {
            option.input_value_x = std::stol(argv[1]);
            option.input_num++;
        }
    }

    if (argc >= 3) {
        if (stdsc::utility::isdigit(argv[2])) {
            option.input_value_y = std::stol(argv[2]);
            option.input_num++;
        }
    }

    if (option.input_num == 1) {
        STDSC_LOG_INFO("input_values (n:%d): x: %ld",
                       option.input_num, option.input_value_x);
    } else {
        STDSC_LOG_INFO("input_values (n:%d): x: %ld, y: %ld",
                       option.input_num, option.input_value_x, option.input_value_y);
    }
}

int32_t init_keys(const std::string& dec_host,
                  const std::string& dec_port,
                  seal::SecretKey& seckey,
                  seal::PublicKey& pubkey,
                  seal::GaloisKeys& galoiskey,
                  seal::EncryptionParameters& params)
{
    fts_user::DecClient dec_client(dec_host.c_str(), dec_port.c_str());
    dec_client.connect();
    
    auto key_id = dec_client.new_keys(seckey);

    dec_client.get_pubkey(key_id, pubkey);
    dec_client.get_galoiskey(key_id, galoiskey);
    dec_client.get_param(key_id, params);

#if defined ENABLE_LOCAL_DEBUG
    {
        std::string dbg_seckey_filename    = "dbg_seckey";
        std::string dbg_pubkey_filename     = "dbg_pukey";
        std::string dbg_galoiskey_filename = "dbg_galoiskey";
        std::string dbg_relinkey_filename  = "dbg_relinkey";
        std::string dbg_params_filename    = "dbg_params";

        seal::RelinKeys relinkey;
        dec_client.get_relinkey(key_id, relinkey);
        
        STDSC_LOG_DEBUG("Write the securet key to file. (key_id:%d, file:%s)",
                        key_id, dbg_seckey_filename.c_str());
        fts_share::seal_utility::write_to_file(dbg_seckey_filename, seckey);
    
        STDSC_LOG_DEBUG("Write the public key to file. (key_id:%d, file:%s)",
                        key_id, dbg_pubkey_filename.c_str());
        fts_share::seal_utility::write_to_file(dbg_pubkey_filename, pubkey);
    
        STDSC_LOG_DEBUG("Write the galois keys to file. (key_id:%d, file:%s)",
                        key_id, dbg_galoiskey_filename.c_str());
        fts_share::seal_utility::write_to_file(dbg_galoiskey_filename, galoiskey);
    
        STDSC_LOG_DEBUG("Write the relin keys to file. (key_id:%d, file:%s)",
                        key_id, dbg_relinkey_filename.c_str());
        fts_share::seal_utility::write_to_file(dbg_relinkey_filename, relinkey);
            
        STDSC_LOG_DEBUG("Write the params to file. (key_id:%d, file:%s)",
                        key_id, dbg_params_filename.c_str());
        fts_share::seal_utility::write_to_file(dbg_params_filename, params);
    }
#endif
    
    return key_id;
}

void compute_one(const int32_t key_id,
                 const int64_t val,
                 const std::string& cs_host,
                 const std::string& cs_port,
                 const seal::PublicKey& pubkey,
                 const seal::GaloisKeys& galoiskey,
                 const seal::EncryptionParameters& params,
                 CallbackParam& callback_param)
{
    STDSC_LOG_INFO("Encrypt input values. (x: %ld)", val);
    fts_share::EncData enc_inputs(params);
    enc_inputs.encrypt(val, pubkey, galoiskey);

#if defined ENABLE_LOCAL_DEBUG
    {
        // save to file
        fts_share::seal_utility::write_to_file("query.txt", enc_inputs.data());
            
        // save to stream
        auto sz = enc_inputs.stream_size();
        stdsc::BufferStream buffstream(sz);
        std::iostream stream(&buffstream);
        enc_inputs.save_to_stream(stream);

        // load from stream
        fts_share::EncData enc_inputs2(params);
        enc_inputs2.load_from_stream(stream);

        // decrypt
        std::vector<int64_t> output_values;
        enc_inputs2.decrypt(*callback_param.seckey, output_values);
        std::cout << "Debug: decrypt query: " << output_values[0] << std::endl;
        //std::cout << "Debug: decrypt query: " << std::flush;
        //for (const auto& v : output_values) {
        //    std::cout << " " << v;
        //}
        //std::cout << std::endl;
    }
#endif
        
    fts_user::CSClient cs_client(cs_host.c_str(), cs_port.c_str(), params);
    cs_client.connect();

    cs_client.send_query(key_id, fts_share::kFuncOne, enc_inputs,
                         callback_func, &callback_param);

    // wait for finish
    usleep(5*1000*1000);
}

void compute_two(const int32_t key_id,
                 const int64_t val_x,
                 const int64_t val_y,
                 const std::string& cs_host,
                 const std::string& cs_port,
                 const seal::PublicKey& pubkey,
                 const seal::GaloisKeys& galoiskey,
                 const seal::EncryptionParameters& params,
                 CallbackParam& callback_param)
{
    STDSC_LOG_INFO("Encrypt input values. (x: %ld, y: %ld)", val_x, val_y);
    std::vector<int64_t> values{val_x, val_y};
    fts_share::EncData enc_inputs(params);
    enc_inputs.encrypt(values, pubkey, galoiskey);

#if defined ENABLE_LOCAL_DEBUG
    {
        fts_share::seal_utility::write_to_file("two-query.txt", enc_inputs.vdata());
    }
#endif
        
    fts_user::CSClient cs_client(cs_host.c_str(), cs_port.c_str(), params);
    cs_client.connect();

    cs_client.send_query(key_id, fts_share::kFuncTwo, enc_inputs,
                         callback_func, &callback_param);

    // wait for finish
    usleep(5*1000*1000);
}


void exec(Option& option)
{
    const char* host = "localhost";

    STDSC_LOG_INFO("decryptor: %s:%s, computation server: %s:%s",
                   host, PORT_DEC_SRV,
                   host, PORT_CS_SRV);

    seal::SecretKey seckey;
    seal::PublicKey pubkey;
    seal::GaloisKeys galoiskey;
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    auto key_id = init_keys(host, PORT_DEC_SRV,
                            seckey, pubkey, galoiskey, params);
    
    CallbackParam callback_param = {&seckey, &params};

    if (option.input_num == 1) {
        compute_one(key_id, option.input_value_x,
                    host, PORT_CS_SRV,
                    pubkey, galoiskey, params, callback_param);
    } else if (option.input_num == 2) {
        compute_two(key_id, option.input_value_x, option.input_value_y,
                    host, PORT_CS_SRV,
                    pubkey, galoiskey, params, callback_param);
        
    }

}

int main(int argc, char* argv[])
{
    STDSC_INIT_LOG();
    try
    {
        Option option;
        init(option, argc, argv);
        STDSC_LOG_INFO("Launched User demo app.");
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
