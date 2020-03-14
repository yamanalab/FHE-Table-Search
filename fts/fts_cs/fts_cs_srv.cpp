#include <sstream>
#include <stdsc/stdsc_server.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_callback_function_container.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_cs/fts_cs_callback_param.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>
#include <fts_cs/fts_cs_srv.hpp>

namespace fts_cs
{

struct CSServer::Impl
{
public:
    Impl(const char* port,
         const char* dec_host,
         const char* dec_port,
         const std::string& LUT_dir,
         stdsc::CallbackFunctionContainer& callback,
         stdsc::StateContext& state,
         const uint32_t max_concurrent_queries,
         const uint32_t max_results,
         const uint32_t result_lifetime_sec)
        : dec_host_(dec_host),
          dec_port_(dec_port),
          calc_manager_(new CalcManager(LUT_dir, max_concurrent_queries, max_results, result_lifetime_sec)),
          param_(new CallbackParam()),
          cparam_(new CommonCallbackParam(*calc_manager_))
    {
        STDSC_LOG_INFO("Initialized computation server with port #%s", port);        
        callback.set_commondata(static_cast<void*>(param_.get()), sizeof(*param_));
        callback.set_commondata(static_cast<void*>(cparam_.get()), sizeof(*cparam_),
                                stdsc::CommonDataKind_t::kCommonDataOnAllConnection);
        server_ = std::make_shared<stdsc::Server<>>(port, state, callback);
    }

    ~Impl(void) = default;


    void start()
    {
        const bool enable_async_mode = true;
        server_->start(enable_async_mode);

        const uint32_t thread_num = 2;
        calc_manager_->start_threads(thread_num, dec_host_, dec_port_);
    }

    void stop(void)
    {
        server_->stop();
    }

    void wait(void)
    {
        server_->wait();
    }


private:
    std::string dec_host_;
    std::string dec_port_;
    std::shared_ptr<CalcManager> calc_manager_;
    std::shared_ptr<CallbackParam> param_;
    std::shared_ptr<CommonCallbackParam> cparam_;
    std::shared_ptr<stdsc::Server<>> server_;
};

CSServer::CSServer(const char* port,
                   const char* dec_host,
                   const char* dec_port,
                   const std::string &LUT_dir,
                   stdsc::CallbackFunctionContainer &callback,
                   stdsc::StateContext &state,
                   const uint32_t max_concurrent_queries,
                   const uint32_t max_results,
                   const uint32_t result_lifetime_sec)
    : pimpl_(new Impl(port, dec_host, dec_port,
                      LUT_dir, callback, state,
                      max_concurrent_queries,
                      max_results,
                      result_lifetime_sec))
{
}

void CSServer::start()
{
    STDSC_LOG_INFO("Start computation server.");
    pimpl_->start();
}

void CSServer::stop(void)
{
    STDSC_LOG_INFO("Stop computation server.");
    pimpl_->stop();
}

void CSServer::wait(void)
{
    STDSC_LOG_INFO("Waiting for computation server to stop.");
    pimpl_->wait();
}


} /* namespace fts_cs */
