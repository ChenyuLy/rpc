#ifndef ROCKET_NET_RPC_RPC_CONTROLLER_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_CONTROLLER_DISPATCHER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>
#include "rocket/net/tcp/net_addr.h"

namespace rocket
{
    class RpcController : public google::protobuf::RpcController
    {
    public:
        RpcController(){};
        ~RpcController(){};

        virtual void Reset() ;

        virtual bool Failed() const ;

        virtual std::string ErrorText() const ;

        virtual void StartCancel() ;

        virtual void SetFailed(const std::string &reason) ;

        virtual bool IsCanceled() const ;

        virtual void NotifyOnCancel( google::protobuf::Closure *callback) ;

        void SetError(int32_t errorcode,const std::string error_info);

        int32_t GetErrorCode();

        std::string GetErrorInfo();

        void SetReqID(const std::string & req_id);

        std::string GetReqId();

        void SetLocalAddr(NetAddr::s_ptr addr);
        void SetPeerAddr(NetAddr::s_ptr addr);

        NetAddr::s_ptr GetLocalAddr();
        NetAddr::s_ptr GetPeerAddr();

        void setTimeout(int timeout);
        int GetTimeout();

    private:
        int32_t m_error_code{0};
        std::string m_error_info;
        std::string m_req_id;

        bool m_is_failed{false};
        bool m_is_cancled{false};

        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;

        int m_timeout{1000}; // ms
    };

} // namespace rocket

#endif