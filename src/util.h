#pragma once

#include <string>

namespace util {

/**
 * throws on error. Calls strerror() on err.
 * appends what.
 */
void checkReturn(std::string what, int err);

} // namespace
