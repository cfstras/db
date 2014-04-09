#include "util.h"

#include <errno.h>
#include <string.h>

#include <stdexcept>

using namespace std;

namespace util {

void checkReturn(string what, int err) {
	if (err != 0) {
		throw runtime_error("Error while " + what + ": " + strerror(err));
	}
}

}
