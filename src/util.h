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

inline static uint16_t chunkId(uint64_t pageId) {
	return (pageId & 0xffff000000000000LL) >> 6*8;
}

} // namespace

typedef uint64_t TID;

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)


// "System-agnostic"
#ifdef __APPLE__
int posix_fallocate(int fd, off_t offset, off_t len);
#endif


class Exception: public std::exception
{
public:
	explicit Exception(const char* message);

	explicit Exception(const std::string& message);

	virtual ~Exception() throw ();

	virtual const char* what() const throw ();

protected:
	std::string msg_;
};
