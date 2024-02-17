#pragma once

#ifdef BIN_CPP_EXPORTS
#define BIN_CPP __declspec(dllexport)
#else
#define BIN_CPP __declspec(dllimport)
#endif

#include <iostream>

extern "C" BIN_CPP void BinaryCpp(uint8_t * data, int size, int value);