#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

void putc(char c);

class QUart
{
    public:
        void init();

    private:
};

template<class T>
inline std::string format_binary(T x)
{
    char b[sizeof(T)*9+1] = { 0 };
    char* p = b + sizeof(T) * 9;

    for(uint8_t z=0; z<sizeof(T)*8; z++){
        if(z%8==0) *--p = ' ';
        *--p = ((x>>z) & 1) ? '1' : '0';
    }

    return std::string(b);
}

// template<class T>
// inline std::string format_binary(T x)
// {
//     char b[sizeof(T)*8+1] = { 0 };
//
//     for(size_t z=0; z<sizeof(T)*8; z++)
//         b[sizeof(T)*8-1-z] = ((x>>z) & 0x1) ? '1' : '0';
//
//     return std::string(b);
// }
