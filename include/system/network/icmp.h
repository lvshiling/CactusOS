#ifndef __CACTUSOS__SYSTEM__NETWORK__ICMP_H
#define __CACTUSOS__SYSTEM__NETWORK__ICMP_H

#include <common/types.h>
#include <system/network/ipv4.h>

namespace CactusOS
{
    namespace system
    {
        struct InternetControlMessageProtocolMessage
        {
            common::uint8_t type;
            common::uint8_t code;
            
            common::uint16_t checksum;
            common::uint32_t data;

        } __attribute__((packed));
        
        class InternetControlMessageProtocol
        {
        private:
            NetworkManager* backend;
        public:
            InternetControlMessageProtocol(NetworkManager* backend);
            
            void OnInternetProtocolReceived(common::uint32_t srcIP, common::uint32_t dstIP,
                                            common::uint8_t* payload, common::uint32_t size);
            void RequestEchoReply(common::uint32_t ip);
        };
    }
}

#endif