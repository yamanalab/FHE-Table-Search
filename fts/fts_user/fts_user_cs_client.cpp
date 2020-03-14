#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_user2csparam.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_cs2userparam.hpp>
#include <fts_user/fts_user_result_thread.hpp>
#include <fts_user/fts_user_cs_client.hpp>

namespace fts_user
{

struct ResultCallback
{
    std::shared_ptr<ResultThread> thread;
    ResultThreadParam param;
};
    
struct CSClient::Impl
{
    Impl(const char* host, const char* port,
         const seal::EncryptionParameters& enc_params)
        : host_(host),
          port_(port),
          enc_params_(enc_params),
          client_()
    {
    }

    ~Impl(void)
    {
        disconnect();
    }

    void connect(const uint32_t retry_interval_usec,
                 const uint32_t timeout_sec)
    {
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
    }

    void disconnect(void)
    {
        client_.close();
    }

    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs)
    {
        fts_share::PlainData<fts_share::User2CsParam> splaindata;
        fts_share::User2CsParam user2csparam {key_id, static_cast<fts_share::FuncNo_t>(func_no)};
        splaindata.push(user2csparam);

        auto sz = (splaindata.stream_size()
                   + fts_share::seal_utility::stream_size(enc_params_)
                   + enc_inputs.stream_size());
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);
        
        splaindata.save_to_stream(stream);
        seal::EncryptionParameters::Save(enc_params_, stream);
        enc_inputs.save_to_stream(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadQuery, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        fts_share::PlainData<int32_t> rplaindata;
        rplaindata.load_from_stream(rstream);

        return rplaindata.data();
    }

    void recv_results(const int32_t query_id, bool& status, fts_share::EncData& enc_result)
    {
        fts_share::PlainData<int32_t> splaindata;
        splaindata.push(query_id);

        auto sz = (splaindata.stream_size()
                   + fts_share::seal_utility::stream_size(enc_params_));
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save_to_stream(stream);
        seal::EncryptionParameters::Save(enc_params_, stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(fts_share::kControlCodeUpDownloadResult, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);

        fts_share::PlainData<fts_share::Cs2UserParam> rplaindata;
        rplaindata.load_from_stream(rstream);
        auto& cs2userparam = rplaindata.data();
        status = cs2userparam.result == fts_share::kCsCalcResultSuccess;

        if (status) {
            enc_result.load_from_stream(rstream);
#if defined ENABLE_LOCAL_DEBUG
            fts_share::seal_utility::write_to_file("result.txt", enc_result.data());
#endif
        }
    }

    void wait(const int32_t query_id) const
    {
        if (cbmap_.count(query_id)) {
            auto& rcb = cbmap_.at(query_id);
            rcb.thread->wait();
        }
    }

    const char* host_;
    const char* port_;
    const seal::EncryptionParameters& enc_params_;
    stdsc::Client client_;
    std::unordered_map<int32_t, ResultCallback> cbmap_;
};

CSClient::CSClient(const char* host, const char* port,
                   const seal::EncryptionParameters& enc_params)
    : pimpl_(new Impl(host, port, enc_params))
{
}

void CSClient::connect(const uint32_t retry_interval_usec,
                       const uint32_t timeout_sec)
{
    STDSC_LOG_INFO("Connect to computation server.");
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void CSClient::disconnect(void)
{
    STDSC_LOG_INFO("Disconnect from computation server.");
    pimpl_->disconnect();
}

int32_t CSClient::send_query(const int32_t key_id, const int32_t func_no,
                             const fts_share::EncData& enc_inputs) const
{
    STDSC_LOG_INFO("Send query: sending query to computation server. (key_id: %d, func_no:%d)",
                   key_id, func_no);
    auto query_id = pimpl_->send_query(key_id, func_no, enc_inputs);
    STDSC_LOG_INFO("Send query: received query ID (#%d)", query_id);
    return query_id;
}

int32_t CSClient::send_query(const int32_t key_id, const int32_t func_no,
                             const fts_share::EncData& enc_inputs,
                             cbfunc_t cbfunc,
                             void* cbfunc_args) const
{
    int32_t query_id = pimpl_->send_query(key_id, func_no, enc_inputs);
    STDSC_LOG_INFO("Set callback function for query #%d", query_id);
    set_callback(query_id, cbfunc, cbfunc_args);
    return query_id;
}

void CSClient::recv_results(const int32_t query_id, bool& status, fts_share::EncData& enc_result) const
{
    STDSC_LOG_INFO("Waiting for query #%d results ...", query_id);
    pimpl_->recv_results(query_id, status, enc_result);
}

void CSClient::set_callback(const int32_t query_id, cbfunc_t func, void* args) const
{
    ResultCallback rcb;
    rcb.thread = std::make_shared<ResultThread>(*this, pimpl_->enc_params_, func, args);
    rcb.param  = {query_id};
    pimpl_->cbmap_[query_id] = rcb;
    pimpl_->cbmap_[query_id].thread->start(pimpl_->cbmap_[query_id].param);
}

void CSClient::wait(const int32_t query_id) const
{
    pimpl_->wait(query_id);
}

} /* namespace fts_user */
