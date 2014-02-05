
#include <VFSPP/system.hpp>

#include <boost/filesystem/fstream.hpp>

#include "gtest/gtest.h"

using namespace vfspp;
using namespace vfspp::system;

using namespace boost;

TEST(PhysicalEntryTest, FileGetType)
{
	PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system/test1.txt");

	IFileSystemEntry* rootDir = fs.getRootEntry();

	ASSERT_TRUE(rootDir->getType() == vfspp::FILE);
}

TEST(PhysicalEntryTest, OpenRead)
{
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system/test1.txt");

		PhysicalEntry* rootDir = fs.getRootEntry();

		boost::shared_ptr<std::streambuf> buffer = rootDir->open(IFileSystemEntry::MODE_READ);
		std::istream stream(buffer.get());

		std::string content;
		content.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

		ASSERT_STREQ("TestTestTest", content.c_str());
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system/test1.txt");
		fs.setAllowedOperations(0);

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->open(IFileSystemEntry::MODE_READ), vfspp::InvalidOperationException);
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->open(IFileSystemEntry::MODE_READ), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, OpenWrite)
{
	using namespace boost::filesystem;

	{
		const char* testText = "TestTestTest";

		path filePath(TEST_WRITE_DIR "/system/test1.txt");
		// This should create the file
		boost::filesystem::ofstream(filePath).close();
		
		PhysicalFileSystem fs(filePath.generic_string());

		PhysicalEntry* rootDir = fs.getRootEntry();
		std::string content;

		{
			boost::shared_ptr<std::streambuf> buffer = rootDir->open(IFileSystemEntry::MODE_WRITE);

			std::ostream stream(buffer.get());
			stream << testText;
			stream.flush();
		}
		{
			boost::filesystem::ifstream in(filePath);

			content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

			in.close();
		}

		ASSERT_STREQ(testText, content.c_str());

		remove(filePath);
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system/test1.txt");
		fs.setAllowedOperations(0);

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->open(IFileSystemEntry::MODE_WRITE), vfspp::InvalidOperationException);
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->open(IFileSystemEntry::MODE_WRITE), vfspp::InvalidOperationException);
	}
}
