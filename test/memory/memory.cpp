
#include <gtest/gtest.h>

#include <VFSPP/memory.hpp>

#include "globals.hpp"

using namespace vfspp;
using namespace vfspp::memory;

using namespace boost;

TEST(MemoryTest, TestSupportedOperations)
{
	MemoryFileSystem fs;

	ASSERT_EQ(OP_READ, fs.supportedOperations());
}

TEST(MemoryTest, TestAddChild)
{
	{
		MemoryFileSystem fs;
		MemoryFileEntry* rootEntry = fs.getRootEntry();

		const char* testData = "TestTestTest";
		shared_ptr<MemoryFileEntry> newEntry = rootEntry->addChild("Test", vfspp::FILE, 1234,
			reinterpret_cast<void*>(const_cast<char*>(testData)), strlen(testData));

		ASSERT_EQ(1, rootEntry->numChildren());

		ASSERT_STREQ("Test", newEntry->getPath().c_str());
		ASSERT_EQ(vfspp::FILE, newEntry->getType());
		ASSERT_EQ(1234, newEntry->lastWriteTime());

		boost::shared_ptr<std::streambuf> buffer = newEntry->open(IFileSystemEntry::MODE_READ);
		std::istream stream(buffer.get());

		std::string content;
		content.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

		ASSERT_STREQ("TestTestTest", content.c_str());
	}
}
