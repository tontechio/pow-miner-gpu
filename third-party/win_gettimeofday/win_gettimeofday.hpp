
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdint.h>
#include <WinSock2.h>

/*
typedef struct timeval {
  long tv_sec;
  long tv_usec;
} timeval;
*/

int gettimeofday(struct timeval *tp, struct timezone *tzp);

