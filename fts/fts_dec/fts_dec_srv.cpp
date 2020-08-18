#include <sstream>
#include <stdsc/stdsc_server.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_dec/fts_dec_srv.hpp>

namespace fts_dec
{

struct DecServer::Impl
{
public:

    Impl(const char* port,
         stdsc::CallbackFunctionContainer& callback,
         stdsc::StateContext& state)
        : server_(new stdsc::Server<>(port, state, callback)),
          state_(state)
    {
        STDSC_LOG_INFO("Initialized decryptor server with port #%s", port);
    }

    ~Impl(void) = default;


    void start(void)
    {
        bool enable_async_mode = true;
        server_->start(enable_async_mode);
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
    std::shared_ptr<stdsc::Server<>> server_;
    stdsc::StateContext& state_;
};


DecServer::DecServer(const char* port,
                     stdsc::CallbackFunctionContainer& callback,
                     stdsc::StateContext& state)
    : pimpl_(new Impl(port, callback, state))
{
}

void DecServer::start(void)
{
    STDSC_LOG_INFO("Start decryptor server.");
    pimpl_->start();
}

void DecServer::stop(void)
{
    STDSC_LOG_INFO("Stop decryptor server.");
    pimpl_->stop();
}

void DecServer::wait(void)
{
    STDSC_LOG_INFO("Waiting for decryptor server to stop.");
    pimpl_->wait();
}


} /* namespace fts_dec */
