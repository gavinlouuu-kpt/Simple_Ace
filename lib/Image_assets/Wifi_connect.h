#include <pgmspace.h>
#ifndef Wifi_connect_h
#define Wifi_connect_h
const uint16_t Wifi_true_w = 20;
const uint16_t Wifi_true_h = 19;

const unsigned short Wifi_true[380] PROGMEM = {
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xef3b, 0xd698, 0xd678, 0xd699, 0xffbd, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xffbd, 0xc5f6, 0x738d, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0861, 0x4a49, 0xa513, 0xef3b, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xffdd, 0xdeda, 0x5289, 0x0861, 0x3165, 0x738d, 0xbdd6, 0xe71b, 0xef5c, 0xd698, 0xa4f2, 0x4a48, 0x18c3, 0x18c3, 0xa513, 0xffbd, 0xEF1A, 0xEF1A,
    0xEF1A, 0xffdd, 0xc5f7, 0x18e3, 0x2124, 0xb575, 0xf77c, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xffbd, 0xdeda, 0x630b, 0x0861, 0x6b4c, 0xf79d, 0xEF1A,
    0xEF1A, 0xd699, 0x18a2, 0x5269, 0xe71a, 0xEF1A, 0xEF1A, 0xEF1A, 0xe6fa, 0xb595, 0xb574, 0xce37, 0xf79d, 0xEF1A, 0xEF1A, 0xEF1A, 0xa4f3, 0x0861, 0x736d, 0xEF1A,
    0xEF1A, 0xdeb9, 0x7bae, 0xf77c, 0xEF1A, 0xf77c, 0x9cd2, 0x39a6, 0x0000, 0x0841, 0x0861, 0x0000, 0x1082, 0x7bae, 0xdeb9, 0xEF1A, 0xEF1A, 0xbdd6, 0x8c50, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xe6fa, 0x39c6, 0x1082, 0x5289, 0xc617, 0xef5b, 0xef5c, 0xe6fa, 0x9470, 0x2124, 0x10a2, 0x9cb2, 0xffbd, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0x4228, 0x2124, 0xce37, 0xffbd, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xef3b, 0x6b4c, 0x0841, 0xb575, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xb595, 0xdeda, 0xEF1A, 0xEF1A, 0xf75c, 0xb574, 0xad33, 0xd699, 0xEF1A, 0xEF1A, 0xff9d, 0xad54, 0xe71b, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xffbd, 0x9cd2, 0x1082, 0x1081, 0x10a2, 0x0020, 0x4a48, 0xe6da, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xd678, 0x10a2, 0x7bae, 0xef1b, 0xef3b, 0xb5b5, 0x2945, 0x632c, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xffbd, 0xd699, 0xffdd, 0xEF1A, 0xEF1A, 0xEF1A, 0xe71b, 0xef3b, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xef3b, 0xd699, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0x2945, 0x0861, 0xc617, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0x5aeb, 0x2965, 0xdeb9, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A,
    0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xffbd, 0xf75c, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A, 0xEF1A};
#endif