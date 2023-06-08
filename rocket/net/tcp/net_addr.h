#ifndef ROCKET_NET_TCP_TNET_ADDR_H
#define ROCKET_NET_TCP_TNET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
namespace rocket
{
    class NetAddr
    {
    private:
    public:
        typedef std::shared_ptr<NetAddr> s_ptr;
        virtual sockaddr *getSocketAddr() = 0;
        virtual socklen_t getSocketLen() = 0;
        virtual int intGetFamily() = 0;
        virtual std::string toString() = 0;
        virtual bool checkVaile() = 0;
    };

    class IPNetAddr : public NetAddr
    {
    private:
            std::string m_ip;
        uint16_t m_port{0};

        sockaddr_in m_addr;
    public:

        sockaddr *getSocketAddr();
        socklen_t getSocketLen();
        int intGetFamily();
        std::string toString();
        bool checkVaile();

        IPNetAddr(const std::string &ip, uint16_t port);
        IPNetAddr(const std::string addr);
        IPNetAddr(sockaddr_in addr);
    };

} // namespace socket

#endif