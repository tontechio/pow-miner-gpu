
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdint.h>
#include <WinSock2.h>

typedef unsigned int useconds_t;

void usleep(__int64 usec);


