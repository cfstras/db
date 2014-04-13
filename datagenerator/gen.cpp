#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>

using namespace std;

class RandomLong
{
   /// The state
   uint64_t state;

   public:
   /// Constructor
   explicit RandomLong(uint64_t seed=88172645463325252ull) : state(seed) {}

   /// Get the next value
   uint64_t next() { state^=(state<<13); state^=(state>>7); return (state^=(state<<17)); }
};

void flushBuffer(vector<uint64_t> &buffer, int fd);

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "usage: " << argv[0] << " <file name> <number of elements>" << std::endl;
		return -1;
	}
	RandomLong rand;
	unsigned n = atoi(argv[2]);
	if (n==0) {
		std::cerr << "invalid length: " << argv[2] << std::endl;
		return -1;
	}
	int fd, ret;
	if ((fd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
		std::cerr << "cannot open file '" << argv[1] << "': " << strerror(errno) << std::endl;
		return -1;
	}
	cout << "allocating space" << endl;
	if ((ret = posix_fallocate(fd, 0, n*sizeof(uint64_t))) != 0)
		std::cerr << "warning: could not allocate file space: " << strerror(ret) << std::endl;

	cout << "writing" << endl;
	uint64_t percent = 0;
	vector<uint64_t> buffer;
	buffer.reserve(1024);

	for (unsigned i=0; i<n; ++i) {
		buffer.push_back(rand.next());
		if (buffer.size() == 1024) {
			flushBuffer(buffer, fd);
		}
		if ((i*100 / n) > percent) {
			percent = i*100 / n;
			std::cout << percent << "%" << endl;
		}
	}
	flushBuffer(buffer, fd);
	cout << n << " numbers written." << endl;
	close(fd);
	return 0;
}
void flushBuffer(vector<uint64_t> &buffer, int fd) {
	if (write(fd, &buffer[0], sizeof(uint64_t) * buffer.size()) < 0) {
		std::cout << "error writing to output file" << ": " << strerror(errno) << std::endl;
	}
	buffer.resize(0);
	buffer.reserve(1024);
}
