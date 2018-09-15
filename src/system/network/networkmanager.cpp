#include <system/network/networkmanager.h>

using namespace CactusOS;
using namespace CactusOS::common;
using namespace CactusOS::core;
using namespace CactusOS::system;

void printf(char*);
void printfHex16(uint16_t);

NetworkManager::NetworkManager(NetworkDriver* net_device)
{
    this->netDevice = net_device;
    this->netDevice->NetManager = this;
}
void NetworkManager::StartNetwork(core::PIT* pit)
{
    //Initialize Handlers
    printf("Adding network handlers\n");
    printf("    -> ARP\n");
    this->arp = new ARPProtocol(this, pit);
}
void NetworkManager::HandleEthernetPacket(common::uint8_t* packet, common::uint32_t size)
{
    EtherFrameHeader* frame = (EtherFrameHeader*)packet;

    if(frame->dstMAC_BE == Convert::ByteSwap(this->netDevice->GetMacAddress()) || frame->dstMAC_BE == MAC_BROADCAST)
    {
        switch(Convert::ByteSwap(frame->etherType_BE))
        {
            case ETHERNET_TYPE_ARP:
                {
                    if(this->arp != 0)
                        this->arp->HandlePacket(packet + sizeof(EtherFrameHeader), size - sizeof(EtherFrameHeader));
                }
                break;
            case ETHERNET_TYPE_IP:
                {

                }
                break;
            default:
                printf("Unkown Ethernet packet type: "); printfHex16(Convert::ByteSwap(frame->etherType_BE)); printf("\n");
        }
    }
}
void NetworkManager::SendEthernetPacket(common::uint64_t dstMAC, common::uint16_t etherType, common::uint8_t* buffer, common::uint32_t size)
{
    if(size > 1518 - sizeof(EtherFrameHeader))
    {
        printf("Packet to big to send, so packet is ignored!\n");
        return;
    }
    else if(size < 64 - sizeof(EtherFrameHeader))
        size = 64 - sizeof(EtherFrameHeader); //Add extra bytes

    uint8_t* buffer2 = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(EtherFrameHeader) + size);
    MemoryOperations::memset(buffer2, 0, sizeof(EtherFrameHeader) + size);
    EtherFrameHeader* frame = (EtherFrameHeader*)buffer2;
    
    frame->dstMAC_BE = Convert::ByteSwap(dstMAC); //Byteswap MAC
    frame->srcMAC_BE = Convert::ByteSwap(netDevice->GetMacAddress()); //Byteswap MAC
    frame->etherType_BE = Convert::ByteSwap(etherType);

    uint8_t* src = buffer;
    uint8_t* dst = buffer2 + sizeof(EtherFrameHeader);
    for(uint32_t i = 0; i < size; i++)
        dst[i] = src[i];
    
    netDevice->SendData(buffer2, size + sizeof(EtherFrameHeader));
    MemoryManager::activeMemoryManager->free(buffer2);  
}
uint48_t NetworkManager::GetMACAddress()
{
    return this->netDevice->GetMacAddress();
}
uint32_t NetworkManager::GetIPAddress()
{
    return NetTools::MakeIP(10, 0, 2, 15); //TODO: Replace by dhcp
}