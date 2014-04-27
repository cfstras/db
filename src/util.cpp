#include "util.h"

#include <errno.h>
#include <string.h>

#include <iostream>
#include <stdexcept>

using namespace std;

namespace util {

void checkReturn(string what, int err) {
	if (err != 0) {
		err = err == -1 ? errno : err;
		auto str = "Error while " + what + ": " + strerror(err);
		throw runtime_error(str);
	}
}

} // namespace

// "System-agnostic"
#ifdef __APPLE__
int posix_fallocate(int fd, off_t offset, off_t len) {return 0;}
#endif
