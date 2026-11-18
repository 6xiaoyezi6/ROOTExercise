#include <iostream>
#include <iomanip>

// 十进制转十六进制
void DecToHex(unsigned int dec)
{
    std::cout << "十六进制: 0x"
              << std::uppercase << std::hex << dec
              << std::dec << std::nouppercase << std::endl;
}

// 十六进制转十进制(要加0x前缀)
void HexToDec(unsigned int hex)
{
    std::cout << "十进制: " << std::dec << hex << std::endl;
}

// 二进制转十进制(要加0b前缀)
void BinaryToDec(unsigned int binary)
{
    std::cout << "十进制: " << std::dec << binary << std::endl;
}

// 十进制转二进制
void DecToBinary(unsigned int dec)
{
    std::cout << "二进制: 0b"
              << std::bitset<16>(dec) << std::endl;
}