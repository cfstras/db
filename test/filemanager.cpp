#include <gtest/gtest.h>

#include <fstream>

#include "filemanager.h"

using namespace std;

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

TEST(FileManagerTest, Path) {
	FileManager fm("test_data");
	int fd = fm.getFile(66);
	ASSERT_LT(0, fd);
	ifstream wrongFile("test_data0");
	EXPECT_FALSE(wrongFile) << "File found with wrong path (test_data0)";
	ifstream file("test_data/0");
	EXPECT_TRUE(file) << "File not created where it should be (test_data/0)";
}

} // namespace
