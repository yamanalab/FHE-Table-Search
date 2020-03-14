#include <unistd.h>
#include <algorithm> // for sort
#include <chrono>
#include <random>
#include <fstream>
#include <sys/types.h>   // for thread id
#include <sys/syscall.h> // for thread id
#include <stdsc/stdsc_log.hpp>
#include <omp.h>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_commonparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_dec_client.hpp>
#include <seal/seal.h>

namespace fts_cs
{

unsigned g_seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 g_generator(g_seed);
    
static bool cmp(std::pair<int64_t, int64_t> a, std::pair<int64_t, int64_t> b)
{
    return a.second < b.second;
}
    
static std::vector<int64_t> get_randomvector(int64_t total)
{
    std::vector<std::pair<int64_t, int64_t> > input;
    std::vector<int64_t> output;
    for (int64_t i=0; i<total; ++i) {
        input.push_back({i, g_generator() % 1000000});
    }
    std::sort(input.begin(), input.end(), cmp);
    for (int64_t i=0; i<total; ++i) {
        output.push_back(input[i].first);
    }
    return output;
}

static void
createLUTforOneInput(const std::vector<std::vector<int64_t>>& LUT,
                     const std::vector<int64_t>& randomVector,
                     std::vector<std::vector<int64_t>>& LUT_input,
                     std::vector<std::vector<int64_t>>& LUT_output,
                     const int64_t l, const int64_t k)
{
    std::vector<int64_t> sub_input;
    std::vector<int64_t> sub_output;
    int64_t total = randomVector.size();
    int64_t row_size = l;
    int64_t index = 0;

    STDSC_LOG_INFO("create new LUT. (l:%ld, k:%ld, total:%ld)",
                   l, k, total);
    
    for (int64_t i=0; i<k; ++i) {
        for(int64_t j=0; j<row_size; ++j) {
            int64_t s = randomVector[index];
            int64_t temp_in = LUT[0][s];
            sub_input.push_back(temp_in);
            int64_t temp_out=LUT[1][s];
            sub_output.push_back(temp_out);
            ++index;
        }
        sub_input.resize(l);
        LUT_input.push_back(sub_input);
        sub_input.clear();
        sub_output.resize(l);
        LUT_output.push_back(sub_output);
        sub_output.clear();
    }
}

static void
createLUTforTwoInput(const std::vector<int64_t>& table_x,
                     const std::vector<int64_t>& table_y,
                     const std::vector<int64_t>& table_out,
                     const std::vector<int64_t>& vi_x,
                     const std::vector<int64_t>& vi_y,
                     const int64_t possible_input_num_two,
                     std::vector<std::vector<int64_t>>& permute_table_x,
                     std::vector<std::vector<int64_t>>& permute_table_y,
                     std::vector<std::vector<int64_t>>& permute_out,
                     const int64_t l,
                     const int64_t k,
                     const int64_t ks)
{
    std::vector<int64_t> per_x, per_y;
    for (int i=0; i<possible_input_num_two; ++i) {
        int64_t tempx = vi_x[i];
        int64_t tempy = vi_y[i];
        per_x.push_back(table_x[tempx]);
        per_y.push_back(table_y[tempy]);
    }

    std::vector<int64_t> sub_per_x, sub_per_y;
    for (int i=0; i<k; ++i) {
        for (int j=0; j<l; ++j) {
            sub_per_x.push_back(per_x[i * l + j]);
            sub_per_y.push_back(per_y[i * l + j]);
        }
        permute_table_x.push_back(sub_per_x);
        sub_per_x.clear();
        permute_table_y.push_back(sub_per_y);
        sub_per_y.clear();
    }

    std::vector<int64_t> per_out, sub_per_out;

    for (int64_t h=0; h<possible_input_num_two; ++h) {
        for(int64_t t=0; t<possible_input_num_two; ++t) {
            int64_t tepx=vi_x[h];
            int64_t tepy=vi_y[t];

            per_out.push_back(table_out[tepx * possible_input_num_two + tepy]);
        }
    }

    STDSC_LOG_INFO("Made permuted output vector.");

    for (int i=0; i<ks; ++i) {
        for(int j=0; j<l; ++j) {
            sub_per_out.push_back(per_out[i * l + j]);
        }
        permute_out.push_back(sub_per_out);
        sub_per_out.clear();
    }
}


    
struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue,
         ResultQueue& out_queue,
         std::vector<std::vector<int64_t>>& LUTin_one,
         std::vector<std::vector<int64_t>>& LUTin_two,
         std::vector<int64_t>& LUTout_two,
         const int64_t possible_input_num_one,
         const int64_t possible_input_num_two,
         const int64_t possible_combination_num_two,
         const std::string& dec_host,
         const std::string& dec_port)
        : in_queue_(in_queue),
          out_queue_(out_queue),
          LUTin_one_(LUTin_one),
          LUTin_two_(LUTin_two),
          LUTout_two_(LUTout_two),
          possible_input_num_one_(possible_input_num_one),
          possible_input_num_two_(possible_input_num_two),
          possible_combination_num_two_(possible_combination_num_two),
          dec_host_(dec_host),
          dec_port_(dec_port)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        auto th_id = syscall(SYS_gettid);
        STDSC_LOG_INFO("Launched calcuration thread. (thread ID: %d)", th_id);
        
        while (!args.force_finish) {

            STDSC_LOG_INFO("[th:%d] Try getting query from QueryQueue.", th_id);

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            bool status = false;
            
            STDSC_LOG_INFO("[th:%d] Get query #%d.", th_id, query_id);

            seal::PublicKey pubkey;
            seal::GaloisKeys galoiskey;
            seal::RelinKeys relinkey;
            seal::EncryptionParameters params(seal::scheme_type::BFV);
            STDSC_LOG_INFO("[th:%d] Start preprocess of query #%d.", th_id, query_id);
            preprocess(query.key_id_, pubkey, galoiskey, relinkey, params);
            STDSC_LOG_INFO("[th:%d] Finish preprocess of query #%d.", th_id, query_id);

            seal::Ciphertext sum_result;
            if (query.func_no_ == fts_share::kFuncTwo) {
                seal::Ciphertext new_PIR_query0, new_PIR_query1, new_PIR_query2;
                std::vector<std::vector<int64_t>> permute_out;
                STDSC_LOG_INFO("[th:%d] Start computationA of query #%d.", th_id, query_id);
                status = computeAforTwoInput(query_id, query,
                                             pubkey, galoiskey, relinkey, params,
                                             permute_out,
                                             new_PIR_query0,
                                             new_PIR_query1,
                                             new_PIR_query2);
                STDSC_LOG_INFO("[th:%d] Finish computationA of query #%d.", th_id, query_id);
                
                if (status) {
                    STDSC_LOG_INFO("[th:%d] Start computationB of query #%d.", th_id, query_id);
                    status = computeBforTwoInput(query_id, query,
                                                 pubkey, galoiskey, relinkey, params,
                                                 permute_out,
                                                 new_PIR_query0,
                                                 new_PIR_query1,
                                                 new_PIR_query2,
                                                 sum_result);
                    STDSC_LOG_INFO("[th:%d] Finish computationB of query #%d.", th_id, query_id);
                }
            } else {
                seal::Ciphertext new_PIR_query, new_PIR_index;
                std::vector<std::vector<int64_t>> LUT_output;
                STDSC_LOG_INFO("[th:%d] Start computationA of query #%d.", th_id, query_id);
                status = computeAforOneInput(query_id, query,
                                             pubkey, galoiskey, relinkey, params,
                                             LUT_output,
                                             new_PIR_query,
                                             new_PIR_index);
                STDSC_LOG_INFO("[th:%d] Finish computationA of query #%d.", th_id, query_id);
                
                if (status) {
                    STDSC_LOG_INFO("[th:%d] Start computationB of query #%d.", th_id, query_id);
                    status = computeBforOneInput(query_id, query,
                                                 pubkey, galoiskey, relinkey, params,
                                                 LUT_output,
                                                 new_PIR_query,
                                                 new_PIR_index,
                                                 sum_result);
                    STDSC_LOG_INFO("[th:%d] Finish computationB of query #%d.", th_id, query_id);
                }
            }
                
            Result result(query_id, status, sum_result);
            out_queue_.push(query_id, result);

            STDSC_LOG_INFO("[th:%d] Set result of query #%d.", th_id, query_id);
        }
    }

    void preprocess(const int32_t key_id,
                    seal::PublicKey& pubkey,
                    seal::GaloisKeys& galoiskey,
                    seal::RelinKeys& relinkey,
                    seal::EncryptionParameters& params)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();
        
        dec_client.get_pubkey(key_id,    pubkey);
        dec_client.get_galoiskey(key_id, galoiskey);
        dec_client.get_relinkey(key_id,  relinkey);
        dec_client.get_param(key_id,     params);

#if defined ENABLE_LOCAL_DEBUG
        {
            fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);
            fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);
            fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
            fts_share::seal_utility::write_to_file("param.txt", params);
        }
#endif
    }
    
    bool computeAforOneInput(const int32_t query_id,
                             const Query& query,
                             const seal::PublicKey& pubkey,
                             const seal::GaloisKeys& galoiskey,
                             const seal::RelinKeys& relinkey,
                             const seal::EncryptionParameters& params,
                             std::vector<std::vector<int64_t>>& LUT_output,
                             seal::Ciphertext& new_PIR_query,
                             seal::Ciphertext& new_PIR_index)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();

        auto& ciphertext_query = query.ctxts_[0];
        auto context = seal::SEALContext::Create(params);

        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);
        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t l = row_size;
        int64_t k = ceil(possible_input_num_one_ / row_size);

        std::vector<int64_t> vi = get_randomvector(possible_input_num_one_);
        
        std::vector<std::vector<int64_t>> LUT_input;
        createLUTforOneInput(LUTin_one_, vi, LUT_input, LUT_output, l, k);

#if defined ENABLE_LOCAL_DEBUG
        //write shifted_output_table in a file
        {
            std::ofstream OutputTable;
            OutputTable.open("queryt");
            for(int i=0; i<k; ++i) {
                for(int j=0; j<l; ++j) {
                    OutputTable << LUT_output[i][j] <<' ';
                }
                OutputTable << std::endl;
            }
            OutputTable.close();
        }
#endif

        std::cout << "  Compute every row of table" << std::endl;
        
        std::vector<seal::Ciphertext> Result;
        for(int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            Result.push_back(tep);
        }

        omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
        #pragma omp parallel for
        for(int64_t i=0; i<k; ++i) {
            seal::Ciphertext res = ciphertext_query;
            seal::Plaintext poly_row;
            batch_encoder.encode(LUT_input[i], poly_row);
            evaluator.sub_plain_inplace(res, poly_row);
            evaluator.relinearize_inplace(res, relinkey);

            std::vector<int64_t> random_value_vec;
            for(size_t sk=0; sk<row_size; ++sk) {
                int64_t random_value = (g_generator() % 5 + 1);
                random_value_vec.push_back(random_value);
            }
            random_value_vec.resize(slot_count);
            seal::Plaintext poly_num;
            batch_encoder.encode(random_value_vec, poly_num);

            evaluator.multiply_plain_inplace(res, poly_num);
            evaluator.relinearize_inplace(res, relinkey);
            Result[i]=res;
        }

#if defined ENABLE_LOCAL_DEBUG
        {
            std::cout << "Saving..." << std::flush;
            std::ofstream outResult;
            outResult.open("queryc", std::ios::binary);
            for(int i=0; i<k; ++i) {
                Result[i].save(outResult);
            }
            outResult.close();
            std::cout << "OK" << std::endl;
        }
#endif

        std::cout << "  Send intermediate resutls to decryptor" << std::endl;
        fts_share::EncData enc_midresult(params, Result);
        fts_share::EncData enc_midresult_dummy(params);

        fts_share::EncData enc_PIRquery(params);
        auto res = dec_client.get_PIRquery(query.func_no_,
                                           query.key_id_,
                                           query_id, 
                                           possible_input_num_one_,
                                           possible_input_num_two_,
                                           possible_combination_num_two_,
                                           enc_midresult,
                                           enc_midresult_dummy,
                                           enc_PIRquery);

        if (res != fts_share::kDecCalcResultSuccess) {
            STDSC_LOG_WARN("  Failed to calcurate PIR queries on decryptor. (errno: %d)",
                           static_cast<int32_t>(res));
            return false;
        }
        
        std::cout << "  Received PIR queries from decryptor" << std::endl;
        
#if defined ENABLE_LOCAL_DEBUG
        {
            auto& queryd = enc_PIRquery.data();
            auto& queryi = enc_PIRindex.data();
            fts_share::seal_utility::write_to_file("queryd", queryd);
            fts_share::seal_utility::write_to_file("queryi", queryi);
        }
#endif

        new_PIR_query = enc_PIRquery.vdata()[0];
        new_PIR_index = enc_PIRquery.vdata()[1];

        return true;
    }

    bool computeAforTwoInput(const int32_t query_id,
                             const Query& query,
                             const seal::PublicKey& pubkey,
                             const seal::GaloisKeys& galoiskey,
                             const seal::RelinKeys& relinkey,
                             const seal::EncryptionParameters& params,
                             std::vector<std::vector<int64_t>>& permute_out,
                             seal::Ciphertext& new_PIR_query0,
                             seal::Ciphertext& new_PIR_query1,
                             seal::Ciphertext& new_PIR_query2)
    {
        if (query.ctxts_.size() < 2) {
            STDSC_THROW_INVARIANT("Invalid input ciphertext number.");
        }
        
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect();

        auto& ciphertext_x = query.ctxts_[0];
        auto& ciphertext_y = query.ctxts_[1];
        auto context = seal::SEALContext::Create(params);

        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);
        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t l = row_size;
        int64_t k = ceil(possible_input_num_two_ / row_size);
        int64_t ks = ceil(possible_combination_num_two_ / row_size);

        std::vector<int64_t> table_x, table_y, table_output;
        for(int i=0; i<possible_input_num_two_; ++i){
            table_x.push_back(LUTin_two_[0][i]);
            table_y.push_back(LUTin_two_[1][i]);
        }

        std::vector<int64_t> vi_x = get_randomvector(possible_input_num_two_);
        std::vector<int64_t> vi_y = get_randomvector(possible_input_num_two_);

        std::vector<std::vector<int64_t>> permute_table_x, permute_table_y;
        createLUTforTwoInput(table_x,
                             table_y,
                             LUTout_two_,
                             vi_x,
                             vi_y,
                             possible_input_num_two_,
                             permute_table_x,
                             permute_table_y,
                             permute_out,
                             l, k, ks);

#if defined ENABLE_LOCAL_DEBUG
        //write shifted_output_table in a file
        {
            std::ofstream OutputTable;
            OutputTable.open("query_table_out");
            for(int i=0; i<ks; ++i) {
                for(int j=0; j<l; ++j) {
                    OutputTable << permute_out[i][j] <<' ';
                }
                OutputTable << std::endl;
            }
            OutputTable.close();
        }
#endif

        std::vector<seal::Ciphertext> result_x, result_y;
        for (int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            result_x.push_back(tep);
            result_y.push_back(tep);
        }

        //thread work
        omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
        #pragma omp parallel for
        for (int64_t i=0; i<k; ++i) {
            seal::Ciphertext res_x = ciphertext_x;
            seal::Plaintext poly_row_x;
            permute_table_x[i].resize(slot_count);
            batch_encoder.encode(permute_table_x[i], poly_row_x);
            evaluator.sub_plain_inplace(res_x, poly_row_x);
            evaluator.relinearize_inplace(res_x, relinkey);

            std::vector<int64_t> random_value_vec1;
            for (size_t sk=0; sk<row_size; ++sk) {
                int64_t random_value=(g_generator() % 5 + 1);
                random_value_vec1.push_back(random_value);
            }
            random_value_vec1.resize(slot_count);
            seal::Plaintext poly_num_x;
            batch_encoder.encode(random_value_vec1, poly_num_x);

            evaluator.multiply_plain_inplace(res_x, poly_num_x);
            evaluator.relinearize_inplace(res_x, relinkey);
            std::cout << "  Size after relinearization: " << res_x.size() << std::endl;
            std::cout << "  Noise budget after relinearizing (dbc = "
                      << relinkey.decomposition_bit_count() << std::endl;
            result_x[i]=res_x;

            seal::Ciphertext res_y = ciphertext_y;
            seal::Plaintext poly_row_y;
            permute_table_y[i].resize(slot_count);
            batch_encoder.encode(permute_table_y[i], poly_row_y);
            evaluator.sub_plain_inplace(res_y, poly_row_y);
            evaluator.relinearize_inplace(res_y, relinkey);

            std::vector<int64_t> random_value_vec2;
            for (size_t sk=0; sk<row_size; ++sk) {
                int64_t random_value=(g_generator()%5+1);
                random_value_vec2.push_back(random_value);
            }
            random_value_vec2.resize(slot_count);
            seal::Plaintext poly_num_y;
            batch_encoder.encode(random_value_vec2, poly_num_y);

            evaluator.multiply_plain_inplace(res_y, poly_num_y);
            evaluator.relinearize_inplace(res_y, relinkey);
            std::cout << "  Size after relinearization: " << res_y.size() << std::endl;
            std::cout << "  Noise budget after relinearizing (dbc = "
                      << relinkey.decomposition_bit_count() << std::endl;
            result_y[i]=res_y;
        }
        
#if defined ENABLE_LOCAL_DEBUG
        {
            std::cout << "Saving..." << std::flush;
            std::ofstream outResult_x, outResult_y;
            outResult_x.open(s1+"_xc1", std::ios::binary);
            outResult_y.open(s1+"_yc1", std::ios::binary);
            for(int i=0; i<k; i++) {
                result_x[i].save(outResult_x);
                result_y[i].save(outResult_y);
            }
            outResult_x.close();
            outResult_y.close();
            std::cout << "OK" << std::endl;
        }
#endif

        std::cout << "  Send intermediate resutls to decryptor" << std::endl;
        fts_share::EncData enc_midresult_x(params, result_x);
        fts_share::EncData enc_midresult_y(params, result_y);
        fts_share::EncData enc_PIRquery(params);
        auto res = dec_client.get_PIRquery(query.func_no_,
                                           query.key_id_,
                                           query_id, 
                                           possible_input_num_one_,
                                           possible_input_num_two_,
                                           possible_combination_num_two_,
                                           enc_midresult_x,
                                           enc_midresult_y,
                                           enc_PIRquery);

        if (res != fts_share::kDecCalcResultSuccess) {
            STDSC_LOG_WARN("  Failed to calcurate PIR queries on decryptor. (errno: %d)",
                           static_cast<int32_t>(res));
            return false;
        }
        
        std::cout << "  Received PIR queries from decryptor" << std::endl;
        
#if defined ENABLE_LOCAL_DEBUG
        {
            fts_share::seal_utility::write_to_file("query_qds2", enc_PIRquery.vdata());
        }
#endif

        new_PIR_query0 = enc_PIRquery.vdata()[0];
        new_PIR_query1 = enc_PIRquery.vdata()[1];
        new_PIR_query2 = enc_PIRquery.vdata()[2];

        return true;
    }
    
    bool computeBforOneInput(const int32_t query_id,
                             const Query& query,
                             const seal::PublicKey& pubkey,
                             const seal::GaloisKeys& galoiskey,
                             const seal::RelinKeys& relinkey,
                             const seal::EncryptionParameters& params,
                             const std::vector<std::vector<int64_t>>& LUT,
                             const seal::Ciphertext& new_PIR_query,
                             const seal::Ciphertext& new_PIR_index,
                             seal::Ciphertext& sum_result)
    {
        auto context = seal::SEALContext::Create(params);

        seal::Encryptor encryptor(context, pubkey);
        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t k = ceil(possible_input_num_one_ / row_size);

        const seal::Ciphertext& new_query = new_PIR_query;
        const seal::Ciphertext& new_index = new_PIR_index;

        std::vector<seal::Ciphertext> res;
        for (int i=0; i<k; ++i) {
            seal::Ciphertext tep;
            res.push_back(tep);
        }

        std::vector<std::vector<int64_t>> tmpLUT(LUT.size());
        std::copy(LUT.begin(), LUT.end(), tmpLUT.begin());
        
        omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
        #pragma omp parallel for
        for (int64_t i=0; i<k; ++i) {
            tmpLUT[i].resize(slot_count);
            seal::Plaintext poly_table_row;
            batch_encoder.encode(tmpLUT[i], poly_table_row);
            seal::Ciphertext temp = new_index;
            evaluator.rotate_rows_inplace(temp, -i, galoiskey);
            evaluator.multiply_inplace(temp, new_query);
            evaluator.relinearize_inplace(temp, relinkey);
            evaluator.multiply_plain_inplace(temp, poly_table_row);
            evaluator.relinearize_inplace(temp, relinkey);
            res[i]=temp;
        }

        sum_result = res[0];
        for(int i=1; i<k; ++i) {
            evaluator.add_inplace(sum_result, res[i]);
            evaluator.relinearize_inplace(sum_result, relinkey);
        }

#if defined ENABLE_LOCAL_DEBUG
        //write Final_result in a file
        {
            std::cout << "Saving final result..." << std::flush;
            std::ofstream Final_result;
            Final_result.open("queryr", std::ios::binary);
            sum_result.save(Final_result);
            Final_result.close();
            std::cout << "OK" << std::endl;
        }
#endif

        return true;
    }

    bool computeBforTwoInput(const int32_t query_id,
                             const Query& query,
                             const seal::PublicKey& pubkey,
                             const seal::GaloisKeys& galoiskey,
                             const seal::RelinKeys& relinkey,
                             const seal::EncryptionParameters& params,
                             const std::vector<std::vector<int64_t>>& permute_out,
                             const seal::Ciphertext& new_PIR_query0,
                             const seal::Ciphertext& new_PIR_query1,
                             const seal::Ciphertext& new_PIR_query2,
                             seal::Ciphertext& sum_result)
    {
        auto context = seal::SEALContext::Create(params);

        seal::Encryptor encryptor(context, pubkey);
        seal::Evaluator evaluator(context);
        seal::BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        size_t row_size = slot_count / 2;
        std::cout << "  Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "  Slot nums = " << slot_count << std::endl;

        int64_t ks= ceil(possible_combination_num_two_ / row_size);

        const seal::Ciphertext& new_query0 = new_PIR_query0;
        const seal::Ciphertext& new_query1 = new_PIR_query1;
        const seal::Ciphertext& new_query2 = new_PIR_query2;

        std::vector<seal::Ciphertext> query_rec, query_sub;
        for (int i=0; i<ks; ++i) {
            seal::Ciphertext temp_rec;
            query_rec.push_back(temp_rec);
        }
        for (size_t i=0; i<row_size; ++i) {
            seal::Ciphertext temp_sub;
            query_sub.push_back(temp_sub);
        }

        std::cout << "  First level threads work" << std::endl;

        omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
        #pragma omp parallel for
        for (size_t i=0; i<row_size; ++i) {
            seal::Ciphertext temp = new_query2;
            evaluator.rotate_rows_inplace(temp, -i, galoiskey);
            evaluator.multiply_inplace(temp, new_query1);
            evaluator.relinearize_inplace(temp, relinkey);
            query_sub[i] = temp;
        }

        std::cout << "  Second level threads work" << std::endl;
        std::cout << "  ks:" << ks << std::endl;

        std::vector<std::vector<int64_t>> tmp_permute_out(permute_out.size());
        std::copy(permute_out.begin(), permute_out.end(), tmp_permute_out.begin());
        
        omp_set_num_threads(FTS_COMMONPARAM_NTHREADS);
        #pragma omp parallel for
        for (int64_t i=0; i<ks; ++i) {
            int64_t ss = i / row_size;
            int64_t kk = i % row_size;
            tmp_permute_out[i].resize(slot_count);
            seal::Plaintext poly_table_row;
            batch_encoder.encode(tmp_permute_out[i], poly_table_row);
            seal::Ciphertext temp1 = query_sub[ss];
            evaluator.rotate_rows_inplace(temp1, -kk, galoiskey);
            evaluator.multiply_inplace(temp1, new_query0);
            evaluator.relinearize_inplace(temp1, relinkey);
            evaluator.multiply_plain_inplace(temp1, poly_table_row);
            evaluator.relinearize_inplace(temp1, relinkey);
            query_rec[i]=temp1;
        }

        sum_result = query_rec[0];
        for (int i=1; i<ks; ++i) {
            evaluator.add_inplace(sum_result, query_rec[i]);
            evaluator.relinearize_inplace(sum_result, relinkey);
        }
        std::cout << "  Size after relinearization: " << sum_result.size() << std::endl;
        std::cout << "  Noise budget after relinearizing (dbc = "
                  << relinkey.decomposition_bit_count() << std::endl;

#if defined ENABLE_LOCAL_DEBUG
        //write Final_result in a file
        {
            std::cout << "Saving final result..." << std::flush;
            std::ofstream Final_result;
            Final_result.open("result", std::ios::binary);
            sum_result.save(Final_result);
            Final_result.close();
            std::cout << "OK" << std::endl;
        }
#endif

        return true;
    }
    
    
    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    const std::vector<std::vector<int64_t>>& LUTin_one_;
    const std::vector<std::vector<int64_t>>& LUTin_two_;
    const std::vector<int64_t>& LUTout_two_;
    const int64_t possible_input_num_one_;
    const int64_t possible_input_num_two_;
    const int64_t possible_combination_num_two_;
    const std::string& dec_host_;
    const std::string& dec_port_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue,
                       ResultQueue& out_queue,
                       std::vector<std::vector<int64_t>>& LUTin_one,
                       std::vector<std::vector<int64_t>>& LUTin_two,
                       std::vector<int64_t>& LUTout_two,
                       const int64_t possible_input_num_one,
                       const int64_t possible_input_num_two,
                       const int64_t possible_combination_num_two,
                       const std::string& dec_host,
                       const std::string& dec_port)
    : pimpl_(new Impl(in_queue, out_queue, LUTin_one, LUTin_two, LUTout_two, 
                      possible_input_num_one,
                      possible_input_num_two,
                      possible_combination_num_two,
                      dec_host, dec_port))
{}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    STDSC_LOG_INFO("Stop calculation thread.");
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace fts_dec */
