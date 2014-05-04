#include <gtest/gtest.h>

#include <filemanager.h>

namespace {

TEST(FileManagerTest, StaticInstance) {
	EXPECT_NE(nullptr, FileManager::instance());
}

TEST(FileManagerTest, OpenFile) {
	auto fm = FileManager::instance();
	ASSERT_NE(nullptr, fm);
	int fd = fm->getFile(66);
	EXPECT_LT(0, fd);
}

TEST(FileManagerTest, Alternative) {
	FileManager fm("test_data");
}

TEST(FileManagerTest, AlternativeOpen) {
	FileManager fm("test_data");
	int fd = fm.getFile(66);
	EXPECT_LT(0, fd);
}

} // namespace
