#pragma once

#include <cstdint>
#if !defined(UINT64_MAX)
#define __STDC_LIMIT_MACROS
# define UINT64_MAX		0xffffffffffffffffULL
#endif

#ifndef PAGE_SIZE
#	define PAGE_SIZE 4096
#endif

#include <string>

namespace util {

/**
 * throws on error. Calls strerror() on err.
 * appends what.
 */
void checkReturn(std::string what, int err);

} // namespace

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
