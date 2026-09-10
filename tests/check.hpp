#pragma once
#include <cstdlib>
#include <iostream>
// Always evaluate checks in Release; never open CRT assertion dialogs in CI.
#ifdef assert
#undef assert
#endif
#define assert(expression) do { if (!(expression)) { std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: " << #expression << '\n'; std::exit(EXIT_FAILURE); } } while(false)
