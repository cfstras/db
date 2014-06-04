#include "metasegment.h"

#include <cassert>

using namespace std;

class membuf : public std::streambuf
{
public:
	membuf( char * mem, size_t size )
	{
		this->setp( mem, mem + size );
		this->setg( mem, 0, mem + size );
	}
	int_type overflow( int_type charval = traits_type::eof() )
	{
		return traits_type::eof();
	}
	int_type underflow( void )
	{
		return traits_type::eof();
	}
	int sync( void )
	{
		return 0;
	}
};

#pragma pack(push)
#pragma pack(1)
struct MetaHeader {
	uint64_t numData;
	char data[0];
};
#pragma pack(pop)

MetaSegment::MetaSegment(shared_ptr<BufferManager> bm) : MetaSegment(bm, false)
{
}

MetaSegment::MetaSegment(shared_ptr<BufferManager> bm, bool init) :
	bufferManager(bm)
{
	BufferFrame *b = &bufferManager->fixPage(0, true);
	frames.push_back(b);
	MetaHeader *header = (MetaHeader*)b->getData();
	if (init) {
		header->numData = 0;
	} else {
		membuf buf((char*)&header->data, PAGE_SIZE - sizeof(MetaHeader));
		istream is(&buf);
		for (uint64_t i = 0; i < header->numData; i++) {
			MetaDatum m;
			is >> m.name;
			is >> m.segmentID;
			is >> m.numPages;
			uint64_t t;
			is >> t;
			SegmentType type = (SegmentType)t;
			assert(type < SegmentType::TYPE_COUNT);
			m.type = type;
			is >> m.payload;
			segments[m.name] = m;
		}
	}
}

MetaSegment::~MetaSegment() {
	MetaHeader *header = (MetaHeader*)frames[0]->getData();
	membuf buf((char*)&header->data, PAGE_SIZE - sizeof(MetaHeader));
	ostream os(&buf);
	header->numData = segments.size();
	for (const auto el : segments) {
		const MetaDatum &m = el.second;
		os << m.name;
		os << m.segmentID;
		os << m.numPages;
		os << (uint64_t)m.type;
		os << m.payload;
	}
	bufferManager->unfixPage(*frames[0], true);
}

bool MetaSegment::put(const MetaDatum &datum) {
	auto it = segments.find(datum.name);
	if (it != segments.end()) {
		return false;
	}
	segments.insert(it, pair<string, MetaDatum>(datum.name, datum));
	return true;
}

MetaDatum MetaSegment::get(string name) {
	return segments[name];
}
