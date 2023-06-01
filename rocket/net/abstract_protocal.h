#ifndef ROCKET_NET_AB_PROTOCAL_H
#define ROCKET_NET_AB_PROTOCAL_H

#include <memory>

namespace rocket
{
    class AbstractProtocal
    {
    private:
        /* data */
    public:
        AbstractProtocal(/* args */);
        ~AbstractProtocal();


        typedef std::shared_ptr<AbstractProtocal> s_ptr;
    };
    

    
} // namespace rocket


#endif