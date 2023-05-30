#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"
#include <string.h>
#include "net_addr.h"

namespace rocket
{

    sockaddr *rocket::IPNetAddr::getSocketAddr()
    {
        return reinterpret_cast<sockaddr *>(&m_addr);
    }
    socklen_t rocket::IPNetAddr::getSocketLen()
    {
        return sizeof(m_addr);
    }
    int rocket::IPNetAddr::intGetFamily()
    {

        return AF_INET;
    }
    std::string rocket::IPNetAddr::toString()
    {

        return (m_ip + ":" + std::to_string(m_port));
    }
    bool IPNetAddr::checkVaile()
    {
        if (m_ip.empty())return false;

        if (m_port <0 || m_port > 65536) return false;

        if(inet_addr(m_ip.c_str()) == INADDR_NONE) return false;

        return true;

    }
    rocket::IPNetAddr::IPNetAddr(const std::string &ip, uint16_t port) : m_ip(ip), m_port(port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }
    rocket::IPNetAddr::IPNetAddr(const std::string addr)
    {
        size_t i = addr.find_first_of(":");
        if (i == addr.npos)
        {
            ERRORLOG("invaild ipv4addr %s ", addr.c_str());
            return;
        }
        m_ip = addr.substr(0, i);
        m_port = std::atoi(addr.substr(i + 1, addr.size() - 1 - i).c_str());

        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }
    rocket::IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr)
    {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr));
        m_port = ntohs(m_addr.sin_port);
    }
}