#include "rpc_channel.h"
#include "rocket/common/msg_id_util.h"
#include "rocket/common/log.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rpc_controller.h"
#include "rocket/common/errcode.h"

namespace rocket
{
    RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr):m_peer_addr(peer_addr)
    {
        m_client  =  std::make_shared<TcpClient>(m_peer_addr);
    }

    RpcChannel::~RpcChannel()
    {
    }

    void RpcChannel::Init(controller_s_ptr controller, message_s_ptr res, message_s_ptr req, closure_s_ptr done)
    {
        if(m_is_init){
            return;
        }
        m_controller = controller;
        m_request = req;
        m_response = res;
        m_closure = done;
        m_is_init = true;
    }

    void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller,
                                const google::protobuf::Message *request,
                                google::protobuf::Message *response,
                                google::protobuf::Closure *done)
    {
        std::shared_ptr< rocket::TinyPBProtocal>  req_protocal = std::make_shared<rocket::TinyPBProtocal>();
        RpcController* my_controller = dynamic_cast<RpcController*>(controller);
        if(my_controller == NULL){
            ERRORLOG("failed callmethod, rpcCpmtroller convert error");
            return;
        }



        if(my_controller->GetMsgId().empty()){
            req_protocal->m_msg_id = MsgIDUtil::GenMsgID();
            my_controller->SetMsgId(req_protocal->m_msg_id);
        } else {
            req_protocal->m_msg_id = my_controller->GetMsgId();
        }

        req_protocal->m_method_name = method->full_name();
        INFOLOG("%s | call method name [%s]",req_protocal->m_msg_id.c_str(),req_protocal->m_method_name.c_str());
        
        if(!m_is_init){
            // ERRORLOG("RpcCannel not init");
            // return;

            std::string err_info = "RpcCannel not init";
            my_controller->SetError(ERROR_CHANNEL_INIT,err_info);
            ERRORLOG("%s | %s,origin requeset [%s] ",req_protocal->m_msg_id.c_str(),err_info.c_str(),request->ShortDebugString().c_str());
            return;
        }
        //requset 序列化
        if(!request->SerializeToString(&(req_protocal->m_pb_data))){
            std::string err_info = "failed to serialize";
            my_controller->SetError(ERROR_FAILED_SERIALIZE,err_info);
            ERRORLOG("%s | %s,origin requeset [%s] ",req_protocal->m_msg_id.c_str(),err_info.c_str(),request->ShortDebugString().c_str());
            return;
        }

        s_ptr channel = shared_from_this();

        // TcpClient client(m_peer_addr);
        channel->getTcpClient()->connect([req_protocal,channel]() mutable {
            RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());
            if(channel->getTcpClient()->getConnectErrorCode() != 0){
                
                my_controller->SetError(channel->getTcpClient()->getConnectErrorCode(),channel->getTcpClient()->getConnectErrorInfo());
                ERRORLOG(" %s | connect error, error code[%d] , error info[%s],peer addr[%s]",req_protocal->m_msg_id.c_str(),my_controller->GetErrorCode()
                ,my_controller->GetErrorInfo().c_str(),channel->getTcpClient()->getPeerAddr()->toString().c_str());
                return;
            }



            channel->getTcpClient()->writeMessage(req_protocal,[req_protocal,channel,my_controller](AbstractProtocal::s_ptr )mutable{
                INFOLOG("%s | ,send rpc requset success. call method name[%s],peer addr[%s],local addr[%s]",
                req_protocal->m_msg_id.c_str(),req_protocal->m_method_name.c_str(),
                channel->getTcpClient()->getPeerAddr()->toString().c_str(),channel->getTcpClient()->getLocalAddr()->toString().c_str());

                channel->getTcpClient()->readMessage(req_protocal->m_msg_id,[channel,my_controller](AbstractProtocal::s_ptr msg) mutable{
                    std::shared_ptr<rocket::TinyPBProtocal> rsp_protocol = std::dynamic_pointer_cast<rocket::TinyPBProtocal>(msg);
                    INFOLOG("%s |, success get rpc response, call method name[%s],peer addr[%s],local addr[%s]",
                    rsp_protocol->m_msg_id.c_str(),rsp_protocol->m_method_name.c_str(),
                    channel->getTcpClient()->getPeerAddr()->toString().c_str(),channel->getTcpClient()->getLocalAddr()->toString().c_str());

                    // RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());
                    // rsp_protocol->m_pb_data
                    if(!(channel->getgResponse()->ParseFromString(rsp_protocol->m_pb_data))){
                        ERRORLOG(" %s | serialize error",rsp_protocol->m_msg_id.c_str());
                        my_controller->SetError(ERROR_FAILED_SERIALIZE,"serialize error");
                        return;
                    }

                    if(rsp_protocol->m_err_code != 0){
                        ERRORLOG("%s |, call rpc method[%s] failed, error code [%d], error info [%s]"
                        ,rsp_protocol->m_msg_id.c_str(),rsp_protocol->m_method_name.c_str(),
                        rsp_protocol->m_err_code,rsp_protocol->m_err_info.c_str());
                        my_controller->SetError(rsp_protocol->m_err_code,rsp_protocol->m_err_info);

                        return ;
                    }
                    INFOLOG("%s | call rpc success,call method name[%s],peer addr[%s],local addr[%s]",rsp_protocol->m_msg_id.c_str(),rsp_protocol->m_method_name.c_str(),
                    channel->getTcpClient()->getPeerAddr()->toString().c_str(),channel->getTcpClient()->getLocalAddr()->toString().c_str())
                    if(channel->getClosure()){
                        channel->getClosure()->Run();
                    }
                    channel.reset();
                });
            });
        });


    }
    google::protobuf::RpcController *RpcChannel::getController()
    {
        return m_controller.get();
    }
    google::protobuf::Message *RpcChannel::getRequest()
    {
        return m_request.get();
    }
    google::protobuf::Message *RpcChannel::getgResponse()
    {
        return m_response.get();
    }
    google::protobuf::Closure *RpcChannel::getClosure()
    {
        return m_closure.get();
    }
    TcpClient *RpcChannel::getTcpClient()
    {
        return m_client.get();
    }
} // namespace rocket
