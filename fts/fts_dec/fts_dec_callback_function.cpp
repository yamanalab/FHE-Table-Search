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

#include <iostream>
#include <cstring>
#include <fstream>
#include <omp.h>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_commonparam.hpp>
#include <fts_share/fts_cs2decparam.hpp>
#include <fts_share/fts_dec2csparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_dec/fts_dec_callback_function.hpp>
#include <fts_dec/fts_dec_callback_param.hpp>
#include <fts_dec/fts_dec_keycontainer.hpp>
#include <fts_dec/fts_dec_state.hpp>

namespace fts_dec
{

// CallbackFunction for new key Request
DEFUN_DOWNLOAD(CallbackFunctionNewKeyRequest)
{
    STDSC_LOG_INFO("Received new key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& param   = cdata_e->param;
    auto& keycont = cdata_a->keycont;

    auto key_id = keycont.new_keys(param);

    fts_share::PlainData<int32_t> plaindata;
    plaindata.push(key_id);

    auto key_id_sz  = plaindata.stream_size();
    auto seckey_sz = keycont.data_size(key_id, KeyKind_t::kKindSecKey);
    auto total_sz  = seckey_sz + key_id_sz;
    
    stdsc::BufferStream buffstream(total_sz);
    std::iostream stream(&buffstream);

    plaindata.save_to_stream(stream);

    seal::SecretKey seckey;
    keycont.get(key_id, KeyKind_t::kKindSecKey, seckey);
    seckey.save(stream);
    
    STDSC_LOG_INFO("Sending new key request ack. (key ID: %d)", key_id);
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataNewKeys, total_sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventNewKeysRequest);
}

// CallbackFunction for Pubkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionPubKeyRequest)
{
    STDSC_LOG_INFO("Received public key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());

    auto sz = keycont.data_size(key_id, KeyKind_t::kKindPubKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::PublicKey pubkey;
    keycont.get(key_id, KeyKind_t::kKindPubKey, pubkey);
    pubkey.save(stream);

    STDSC_LOG_INFO("Sending public key request ack. (key ID: %d)", key_id);
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataPubKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventPubKeyRequest);
}

// CallbackFunction for Galoiskey Request
DEFUN_UPDOWNLOAD(CallbackFunctionGaloisKeyRequest)
{
    STDSC_LOG_INFO("Received galois keys request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());

    auto sz = keycont.data_size(key_id, KeyKind_t::kKindGaloisKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::GaloisKeys galoiskey;
    keycont.get(key_id, KeyKind_t::kKindGaloisKey, galoiskey);
    galoiskey.save(stream);

    STDSC_LOG_INFO("Sending galois keys request ack. (key ID: %d)", key_id);
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataGaloisKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventGaloisKeyRequest);
}

// CallbackFunction for Relinkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionRelinKeyRequest)
{
    STDSC_LOG_INFO("Received relin keys request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());

    auto sz = keycont.data_size(key_id, KeyKind_t::kKindRelinKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::RelinKeys relinkey;
    keycont.get(key_id, KeyKind_t::kKindRelinKey, relinkey);
    relinkey.save(stream);

    
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataRelinKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventRelinKeyRequest);
}

// CallbackFunction for Param Request
DEFUN_UPDOWNLOAD(CallbackFunctionParamRequest)
{
    STDSC_LOG_INFO("Received encryption parameters request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());

    auto sz = keycont.data_size(key_id, KeyKind_t::kKindParam);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::EncryptionParameters params(seal::scheme_type::BFV);
    keycont.get_param(key_id, params);
    seal::EncryptionParameters::Save(params, stream);

    STDSC_LOG_INFO("Sending encryption parameters ack. (key ID: %d)", key_id);
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataParam, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventParamRequest);
}

// CallbackFunction for Delete key Request
DEFUN_DATA(CallbackFunctionDeleteKeyRequest)
{
    STDSC_LOG_INFO("Received delete key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());

    keycont.delete_keys(key_id);
    state.set(kEventDeleteKeysRequest);
}

static std::vector<int64_t> shift_work(const std::vector<int64_t>& query,
                                       const int64_t index,
                                       const int64_t num_slots)
{
    std::vector<int64_t> new_index;
    int64_t size = query.size();
    int64_t temp;

    for (int64_t i=0; i<num_slots; ++i) {
        if ((i+index) >= size) {
            temp = query[i + index - size];
        } else {
            temp = query[i + index];
        }
        new_index.push_back(temp);
    }
    return new_index;
}

static fts_share::DecCalcResult_t
calcPIRqueriesForOneInput(const std::vector<seal::Ciphertext>& midresults,
                          const seal::SecretKey& seckey,
                          const seal::PublicKey& pubkey,
                          const seal::EncryptionParameters& params,
                          const int64_t possible_input_num_one,
                          seal::Ciphertext& new_PIR_query,
                          seal::Ciphertext& new_PIR_index)
{
    STDSC_LOG_INFO("Start calculation of PIR queries for one input.");
    
    auto context = seal::SEALContext::Create(params);

    seal::Encryptor encryptor(context, pubkey);
    seal::Evaluator evaluator(context);
    seal::Decryptor decryptor(context, seckey);

    seal::BatchEncoder batch_encoder(context);
    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
    std::cout << "  Slot nums = " << slot_count << std::endl;

    int64_t l = row_size;
    int64_t k = ceil(possible_input_num_one / row_size);

    const auto& ct_result = midresults;
    std::vector<std::vector<int64_t>> dec_result(k);
    std::vector<seal::Plaintext> poly_dec_result;
    for (int s=0; s<k; ++s) {
        seal::Plaintext ex;
        poly_dec_result.push_back(ex);
    }

    std::cout << "  Decrypting..."<< std::flush;

    omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
    #pragma omp parallel for
    for (int z=0; z<k; ++z) {
        decryptor.decrypt(ct_result[z], poly_dec_result[z]);
        batch_encoder.decode(poly_dec_result[z], dec_result[z]);
    }

    std::cout << "OK" << std::endl;

    std::cout << "  Making PIR-query..." << std::flush;

    int64_t index;
    std::vector<int64_t> new_query;
    int64_t flag = 0;

    for (int i=0; i<k; ++i) {
        for (size_t j=0; j<row_size; ++j) {
            if (dec_result[i][j] == 0 && flag == 0) {
                index = i;
                flag = 1;
                for (size_t kk=0; kk<static_cast<size_t>(l); ++kk) {
                    if (kk == j) {
                        new_query.push_back(1);
                    } else {
                        new_query.push_back(0);
                    }
                }
            }
        }
    }
    if(flag == 0) {
        std::cout << "ERROR: NO FIND INPUT NUMBER!" << std::endl;
        return fts_share::kDecCalcResultErrNoFoundInputMember;
    }
    std::cout << "OK" << std::endl;

    std::vector<int64_t> new_index;
    new_index = shift_work(new_query, index, row_size);
    std::cout << "  index is " << index << std::endl;
    new_query.resize(slot_count);
    new_index.resize(slot_count);

    //encrypt new query
    std::cout << "  Encrypting..." << std::flush;
    seal::Plaintext plaintext_new_PIR_query;
    batch_encoder.encode(new_query, plaintext_new_PIR_query);

    //seal::Ciphertext new_PIR_index;
    seal::Plaintext plaintext_new_PIR_index;
    batch_encoder.encode(new_index, plaintext_new_PIR_index);

    encryptor.encrypt(plaintext_new_PIR_query, new_PIR_query);
    encryptor.encrypt(plaintext_new_PIR_index, new_PIR_index);

    std::cout << "OK" << std::endl;

#if defined ENABLE_LOCAL_DEBUG
    {
        std::cout << "  Saving query..." << std::flush;
        std::ofstream PIRqueryFile;
        PIRqueryFile.open("queryd", std::ios::binary);
        new_PIR_query.save(PIRqueryFile);
        PIRqueryFile.close();

        std::ofstream PIRindexFile;
        PIRindexFile.open("queryi", std::ios::binary);
        new_PIR_index.save(PIRindexFile);
        PIRindexFile.close();
        
        std::cout << "OK" << std::endl;
    }
#endif

    STDSC_LOG_INFO("Finish calculation of PIR queries.");

    return fts_share::kDecCalcResultSuccess;
}

static fts_share::DecCalcResult_t
calcPIRqueriesForTwoInput(const std::vector<seal::Ciphertext>& midresults_x,
                          const std::vector<seal::Ciphertext>& midresults_y,
                          const seal::SecretKey& seckey,
                          const seal::PublicKey& pubkey,
                          const seal::EncryptionParameters& params,
                          const int64_t possible_input_num_two,
                          const int64_t possible_combination_num_two,
                          seal::Ciphertext& new_PIR_query0,
                          seal::Ciphertext& new_PIR_query1,
                          seal::Ciphertext& new_PIR_query2)
{
    STDSC_LOG_INFO("Start calculation of PIR queries for two input.");
    
    auto context = seal::SEALContext::Create(params);

    seal::Encryptor encryptor(context, pubkey);
    seal::Evaluator evaluator(context);
    seal::Decryptor decryptor(context, seckey);

    seal::BatchEncoder batch_encoder(context);
    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
    std::cout << "  Slot nums = " << slot_count << std::endl;

    int64_t l = row_size;
    int64_t k = ceil(possible_input_num_two / row_size);

    const auto& ct_result_x = midresults_x;
    std::vector<std::vector<int64_t>> dec_result_x(k);
    std::vector<seal::Plaintext> poly_dec_result_x;
    for (int s=0; s<k; ++s) {
        seal::Plaintext ex;
        poly_dec_result_x.push_back(ex);
    }

    const auto& ct_result_y = midresults_y;
    std::vector<std::vector<int64_t>> dec_result_y(k);
    std::vector<seal::Plaintext> poly_dec_result_y;
    for (int s=0; s<k; ++s) {
        seal::Plaintext ex;
        poly_dec_result_y.push_back(ex);
    }
    
    std::cout << "  Decrypting..."<< std::flush;

    omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
    #pragma omp parallel for
    for (int z=0; z<k; ++z) {
        decryptor.decrypt(ct_result_x[z], poly_dec_result_x[z]);
        batch_encoder.decode(poly_dec_result_x[z], dec_result_x[z]);
    }
    printf("caca\n");

    omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
    #pragma omp parallel for
    for (int z=0; z<k; ++z) {
        decryptor.decrypt(ct_result_y[z], poly_dec_result_y[z]);
        batch_encoder.decode(poly_dec_result_y[z], dec_result_y[z]);
    }

    std::cout << "OK" << std::endl;
    
    std::cout << "  Making PIR-query..." << std::flush;

    int64_t flag = 0;
    int64_t index_row_x = -1, index_col_x = -1;
    for (int64_t i=0; i<k; ++i) {
        for(size_t j=0; j<row_size; ++j) {
            if (dec_result_x[i][j] == 0 && flag == 0) {
                index_row_x = i;
                index_col_x = j;
                flag = 1;
            }
        }
    }

    if(flag == 0) {
        std::cout << "ERROR: NO FIND INPUT NUMBER!" << std::endl;
        return fts_share::kDecCalcResultErrNoFoundInputMember;
    }

    flag = 0;
    std::vector<int64_t> new_query0, new_query1, new_query2;
    int64_t index_row_y = -1, index_col_y = -1;
    for (int64_t i=0; i<k; ++i) {
        for(size_t j=0; j<row_size; ++j) {
            if (dec_result_y[i][j] == 0 && flag == 0) {
                index_row_y = i;
                index_col_y = j;
                flag = 1;
                for (size_t kk=0; kk<static_cast<size_t>(l); ++kk) {
                    if(kk == j) {
                        new_query0.push_back(1);
                    } else {
                        new_query0.push_back(0);
                    }
                }
            }
        }
    }

    if(flag == 0) {
        std::cout << "ERROR: NO FIND INPUT NUMBER!" << std::endl;
        return fts_share::kDecCalcResultErrNoFoundInputMember;
    }

    std:: cout << "index_row_x:" << index_row_x << ", index_col_x:" << index_col_x
               << ", index_row_y:" << index_row_y << ", index_col_y:" << index_col_y << std::endl;
    
    std::cout << "OK" << std::endl;

    //new_index is new_query left_shift the value of index
    int64_t index1 = (l * index_row_x + index_col_x) * k + index_row_y;
    index1 = index1 % l;
    int64_t index2 = (l * index_row_x + index_col_x) * k + index_row_y;
    index2 = index2 / l;

    new_query1 = shift_work(new_query0 ,index1, row_size);
    new_query2 = shift_work(new_query1, index2, row_size);
    std::cout << "  index1 is " << index1 << std::endl;
    std::cout << "  index2 is " << index2 << std::endl;
    new_query0.resize(slot_count);
    new_query1.resize(slot_count);
    new_query2.resize(slot_count);

    std::cout << "  Encrypting..." << std::flush;

    seal::Plaintext plaintext_new_PIR_query0, plaintext_new_PIR_query1, plaintext_new_PIR_query2;
    batch_encoder.encode(new_query0, plaintext_new_PIR_query0);
    encryptor.encrypt(plaintext_new_PIR_query0, new_PIR_query0);
    batch_encoder.encode(new_query1, plaintext_new_PIR_query1);
    encryptor.encrypt(plaintext_new_PIR_query1, new_PIR_query1);
    batch_encoder.encode(new_query2, plaintext_new_PIR_query2);
    encryptor.encrypt(plaintext_new_PIR_query2, new_PIR_query2);
    
    std::cout << "OK" << std::endl;

#if defined ENABLE_LOCAL_DEBUG
    {
        std::cout << "  Saving query..." << std::flush;
        std::ofstream queryFile;
        queryFile.open("query_qds2", std::ios::binary);
        new_PIR_query0.save(queryFile);
        new_PIR_query1.save(queryFile);
        new_PIR_query2.save(queryFile);
        queryFile.close();
        std::cout << "OK" << std::endl;
    }
#endif

    STDSC_LOG_INFO("Finish calculation of PIR queries.");

    return fts_share::kDecCalcResultSuccess;
}

    
// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionCsMidResult)
{
    STDSC_LOG_INFO("Received intermediate results. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    fts_share::PlainData<fts_share::Cs2DecParam> rplaindata;
    rplaindata.load_from_stream(rstream);
    const auto cs2decparam = rplaindata.data();

    seal::SecretKey seckey;
    seal::PublicKey pubkey;
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    keycont.get(cs2decparam.key_id, KeyKind_t::kKindSecKey, seckey);
    keycont.get(cs2decparam.key_id, KeyKind_t::kKindPubKey, pubkey);
    keycont.get_param(cs2decparam.key_id, params);

    fts_share::EncData enc_midresult_x(params), enc_midresult_y(params);
    enc_midresult_x.load_from_stream(rstream);
    if (cs2decparam.func_no == fts_share::kFuncTwo) {
        enc_midresult_y.load_from_stream(rstream);
    }
#if defined LOCAL_DEBUG
    fts_share::seal_utility::write_to_file("queryc.txt", enc_midresult_x.data());
#endif

    // one input : [0] query,  [1] index
    // two input : [0] query0, [1] query1, [2] query2
    const size_t nPIRqueries = (cs2decparam.func_no == fts_share::kFuncTwo) ? 3 : 2;
    std::vector<seal::Ciphertext> new_PIR_query(nPIRqueries);

    fts_share::DecCalcResult_t res = fts_share::kDecCalcResultNil;
    if (cs2decparam.func_no == fts_share::kFuncTwo) {
        res = calcPIRqueriesForTwoInput(enc_midresult_x.vdata(),
                                        enc_midresult_y.vdata(),
                                        seckey, pubkey, params,
                                        cs2decparam.possible_input_num_two,
                                        cs2decparam.possible_combination_num_two,
                                        new_PIR_query[0],
                                        new_PIR_query[1],
                                        new_PIR_query[2]);
    } else {
        res = calcPIRqueriesForOneInput(enc_midresult_x.vdata(),
                                        seckey, pubkey, params,
                                        cs2decparam.possible_input_num_one,
                                        new_PIR_query[0], new_PIR_query[1]);
    }

    fts_share::Dec2CsParam dec2csparam = {res};
    fts_share::PlainData<fts_share::Dec2CsParam> splaindata;
    splaindata.push(dec2csparam);
    
    fts_share::EncData enc_PIRquery(params, new_PIR_query);
    
    auto sz = splaindata.stream_size() + enc_PIRquery.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save_to_stream(sstream);
    enc_PIRquery.save_to_stream(sstream);
    
    STDSC_LOG_INFO("Sending PIR queries.");
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataCsMidResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventCsMidResult);
}

} /* namespace fts_dec */
