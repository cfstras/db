#pragma once

#include <stdint.h>
#if !defined(UINT64_MAX)
#define __STDC_LIMIT_MACROS
# define UINT64_MAX		0xffffffffffffffffULL
#endif

#include <string>

namespace util {

/**
 * throws on error. Calls strerror() on err.
 * appends what.
 */
void checkReturn(std::string what, int err);

} // namespace
