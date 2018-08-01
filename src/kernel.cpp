#include <system.h>
#include <multiboot/multiboot.h>
#include <common/convert.h>
#include <system/network/udp.h>
#include <system/network/dhcp.h>

using namespace CactusOS;
using namespace CactusOS::core;
using namespace CactusOS::common;
using namespace CactusOS::system;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

void terminal_scroll(){
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    for(int i = 0; i < 25; i++){
        for (int m = 0; m < 80; m++){
            VideoMemory[i * 80 + m] = VideoMemory[(i + 1) * 80 + m];
        }
    }
}

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                volatile uint16_t * where;
                where = (volatile uint16_t *)0xB8000 + (y * 80 + x) ;
                *where = str[i] | ((0 << 4) | (0xB & 0x0F) << 8);
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            //for(y = 0; y < 25; y++)
            //    for(x = 0; x < 80; x++)
            //        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            terminal_scroll();
            x = 0;
            y = 24;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

void PrintKernelStart()
{
    printf("Starting kernel at: ");
    printf(Convert::IntToString(RTC::GetHour())); printf(":"); printf(Convert::IntToString(RTC::GetMinute())); printf(":"); printf(Convert::IntToString(RTC::GetSecond()));
    printf("   ");
    printf(Convert::IntToString(RTC::GetDay())); printf(":"); printf(Convert::IntToString(RTC::GetMonth())); printf(":"); printf(Convert::IntToString(RTC::GetYear()));
    printf("\n");
}

void PrintIP(uint32_t ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    //printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
    printf(Convert::IntToString(bytes[3])); printf(".");
    printf(Convert::IntToString(bytes[2])); printf(".");
    printf(Convert::IntToString(bytes[1])); printf(".");
    printf(Convert::IntToString(bytes[0])); 
}

void HandleTestData(uint8_t* data, uint32_t size)
{
    printf("Received from socket:\n");
    printf((char*)data);
    printf("\n");
}

extern "C" void kernelMain(const multiboot_info_t* mbi, unsigned int multiboot_magic)
{
    PrintKernelStart();
    printf("CMD Parameters: "); printf((char*)mbi->cmdline); printf("\n");

    //Memory manager is needed for the new keyword
    uint32_t* memupper = (uint32_t*)(((uint32_t)mbi) + 8);
    uint32_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    printf("Memory Manager Loaded\n");

    printf("Starting Core\n");
    System::InitCore();

    printf("Starting System\n");
    System::InitSystem();

    if(System::networkManager != 0)
    {
        /*
        uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
        uint32_t gip_be = ((uint32_t)gip4 << 24)
                   | ((uint32_t)gip3 << 16)
                   | ((uint32_t)gip2 << 8)
                   | (uint32_t)gip1;

        UDPSocket* socket = System::networkManager->ipv4Handler->udpHandler->Listen(1234);
        socket->receiveHandle = HandleTestData;

        printf("Our IP: "); PrintIP(Convert::ByteSwap(System::networkManager->IP_BE)); printf("\n");
        while(socket->listening);
        printf("Connected!\n");
        */

        DHCP::EnableDHCP();
    }

    while(1);
}
