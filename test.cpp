#include <iostream>
#include <arpa/inet.h> // htons için gerekli

int main() {
    unsigned short port = 8080;  // 0x1F90 (Hex)
    unsigned short network_port = htons(port);

    // RAM’de saklanma biçimini görmek için baytları gösterelim:
    unsigned char *bytePointer = (unsigned char*)&port;
    std::cout << "Host byte order (memory layout): " << std::hex;
    std::cout << "[ " << (int)bytePointer[0] << " " << (int)bytePointer[1] << " ]" << std::endl;

    bytePointer = (unsigned char*)&network_port;
    std::cout << "Network byte order (memory layout): ";
    std::cout << "[ " << (int)bytePointer[0] << " " << (int)bytePointer[1] << " ]" << std::endl;

    return 0;
}
