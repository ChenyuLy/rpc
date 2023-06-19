#include "rpc_dispatcher.h"
#include "../coder/tinypb_protocol.h"
#include "rocket/common/log.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <rocket/common/errcode.h>
#include "rocket/net/tcp/net_addr.h"
#include "rpc_controller.h"
#include "rocket/net/tcp/tcp_connection.h"

namespace rocket
{
    void RpcDispatcher::dispatch(AbstractProtocal::s_ptr request, AbstractProtocal::s_ptr response,TcpConnection* connection)
    {
        std::shared_ptr<TinyPBProtocal> req_protocol = std::dynamic_pointer_cast<TinyPBProtocal>(request);
        std::shared_ptr<TinyPBProtocal> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocal>(response);
        std::string method_full_name = req_protocol->m_method_name;
        std::string service_name;
        std::string method_name;

        rsp_protocol->m_req_id = req_protocol->m_req_id;
        rsp_protocol->m_method_name = req_protocol->m_method_name;

        if(parseServiceFullName(method_full_name,service_name,method_name)){
            setTinyPBError(rsp_protocol,ERROR_PARSE_SERVICE_NAME,"parse service name error");
            return;
        }

        auto it = m_service_map.find(service_name);
        if(it == m_service_map.end()){
            ERRORLOG("%s | service name[%s] not found",req_protocol->m_req_id.c_str(),service_name.c_str());
            setTinyPBError(rsp_protocol,ERROR_PARSE_SERVICE_NAME,"service not found");
            return;
        }

        service_s_ptr service = (*it).second;
        const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
        if(method == NULL){
            ERRORLOG("%s | method name[%s] not found in service[%s]",req_protocol->m_req_id.c_str(),method_name.c_str(),service_name.c_str());
            setTinyPBError(rsp_protocol,ERROR_PARSE_SERVICE_NAME,"method not found");
            return;
        }
        

        google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();
        //反序列化 ，将pbdata反序列化为req_msg
        if(!req_msg -> ParseFromString(req_protocol->m_pb_data)){
            ERRORLOG("%s | deserilize error ",req_protocol->m_req_id.c_str(),method_name.c_str(),service_name.c_str());
            setTinyPBError(rsp_protocol,ERROR_FAILED_DESERIALIZE,"deserilize fail");
            if(req_msg != NULL) {
                delete req_msg;
                req_msg = NULL;
            }
            return;
        }

        INFOLOG("%s | get rpc request[%s] get rpc request",req_protocol->m_req_id.c_str(),req_msg->ShortDebugString().c_str());

        google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();


        RpcController rpcController;
        // IPNetAddr::s_ptr local_addr = std::make_shared<IPNetAddr>("127.0.0.1",1234);
        rpcController.SetLocalAddr(connection->getLocalAddr());
        rpcController.SetPeerAddr(connection->getPeerAddr());
        rpcController.SetReqID(req_protocol->m_req_id);

        service->CallMethod(method,&rpcController,req_msg,rsp_msg,NULL);

        if(rsp_msg->SerializeToString(&(rsp_protocol->m_method_name))){
            ERRORLOG("%s | serilize error ,origin message [%s]",req_protocol->m_req_id.c_str(), rsp_msg->ShortDebugString().c_str());
            setTinyPBError(rsp_protocol,ERROR_FAILED_SERIALIZE,"serilize error");
            if(req_msg != NULL) {
                delete req_msg;
                req_msg = NULL;
            }
            if(rsp_msg != NULL) {
                delete rsp_msg;
                rsp_msg = NULL;
            }
            return;
        }

        rsp_protocol->m_err_code = 0;
        INFOLOG("%s | dispath success , requesut[%s],response[%s]",req_protocol->m_req_id.c_str(),req_msg->ShortDebugString().c_str(),rsp_msg->ShortDebugString().c_str())
        delete req_msg;
        req_msg = NULL;
        delete rsp_msg;
        rsp_msg = NULL;
        
    }


    void RpcDispatcher::registerService(std::shared_ptr<google::protobuf::Service> service)
    {
        std::string service_name = service->GetDescriptor()->full_name();
        m_service_map[service_name] = service;
    }
    void RpcDispatcher::setTinyPBError(std::shared_ptr<rocket::TinyPBProtocal> msg, int32_t err_code, const std::string err_info)
    {
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();


    }
    bool RpcDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name)
    {
        if(full_name.empty()){
            ERRORLOG("full name empty");
            return false;
        }
        size_t i = full_name.find_first_of(".");
        if(i == full_name.npos){
            ERRORLOG("not find . in full name [%s]",full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0,i);
        method_name = full_name.substr(i+1,full_name.length()-i-1);
        INFOLOG("parse service_name[%s] and method_name [%s]from full name [%s]",service_name.c_str(),method_name.c_str(),full_name.c_str());
        return true;

    }



} // namespace rocket

//1.注册orderservice 对象。
//2.从buffer读取数据，然后decode得到请求的TinyPBProtobol对象 从请求的TinyPBProtobol得到method_name，从orderservice 对象里更具service.method_name 找到方法 func
//3.总该带对于于不顾的 request type以及response type
//4.从请求的TinyPBProtobol将请求体里面的pb_date 反序列化为 request type的一个对象，声明一个空的response type 对象
//5.func（request，response）
//6.将repsonse 对象序列化为pb_date 在塞入道TinyPBProtobol结构体中 结构体中做encode塞入道buffer里面 就会发送回bao而来