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

typedef uint64_t TID;
typedef uint64_t PageID;
typedef uint16_t SegmentID;

namespace util {

/**
 * throws on error. Calls strerror() on err.
 * appends what.
 */
void checkReturn(std::string what, int err);

inline static SegmentID extractSegmentFromPageID(PageID pageId) {
	return (pageId & 0xffff000000000000ULL) >> 12 * 4; // 12 nibbles
}

// Only lower 48bit matter
inline static uint64_t extractPageFromPageID(PageID pageId) {
	return (pageId & 0x0000ffffffffffffULL);
}

inline static SegmentID extractSlotIDFromTID(TID tid) {
	return (tid & 0x000000000000ffffULL);
}

// Only lower 48 bit matter
inline static uint64_t extractPageIDFromTID(TID tid) {
	return (tid & 0xffffffffffff0000ULL) >> 4 * 4; // 4 nibbles
}

} // namespace


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
