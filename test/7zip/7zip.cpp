
#include <VFSPP/7zip.hpp>
#include <globals.hpp>

#include <boost/foreach.hpp>

#include "gtest/gtest.h"

#include <iostream>

using namespace vfspp;
using namespace vfspp::sevenzip;

using namespace vfspp::test;

using namespace boost;


TEST(SevenZipFileEntryTest, NumChildren)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	ASSERT_EQ(7, fs.getRootEntry()->numChildren());
}


TEST(SevenZipFileEntryTest, FillChildren)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	std::vector<shared_ptr<IFileSystemEntry>> children;
	{
		IFileSystemEntry* rootDir = fs.getRootEntry();
		rootDir->listChildren(children);

		ASSERT_EQ(7, children.size());

		ASSERT_TRUE(vectorContainsEntry(children, "test1", DIRECTORY));
		ASSERT_TRUE(vectorContainsEntry(children, "test2", DIRECTORY));
		ASSERT_TRUE(vectorContainsEntry(children, "test3", DIRECTORY));

		ASSERT_TRUE(vectorContainsEntry(children, "test1.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test3.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test4.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test5.txt", vfspp::FILE));
	}
}

TEST(SevenZipFileEntryTest, DeleteChild)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	IFileSystemEntry *root = fs.getRootEntry();

	ASSERT_THROW(root->deleteChild("bar.txt"), vfspp::InvalidOperationException);
}

TEST(SevenZipFileEntryTest, CreateFile)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	IFileSystemEntry *root = fs.getRootEntry();

	ASSERT_THROW(root->createEntry(vfspp::FILE, "bar.txt"), vfspp::InvalidOperationException);
}

TEST(SevenZipFileEntryTest, CreateDirectory)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	IFileSystemEntry *root = fs.getRootEntry();

	ASSERT_THROW(root->createEntry(vfspp::DIRECTORY, "bar.txt"), vfspp::InvalidOperationException);
}

TEST(SevenZipFileEntryTest, OpenRead)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	IFileSystemEntry *root = fs.getRootEntry();

	{
		boost::shared_ptr<IFileSystemEntry> entry = root->getChild("test1.txt");

		boost::shared_ptr<std::streambuf> buffer = entry->open(IFileSystemEntry::MODE_READ);
		std::istream stream(buffer.get());

		std::string content;
		content.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

		ASSERT_STREQ("TestTestTest", content.c_str());
	}
	{
		ASSERT_THROW(root->open(IFileSystemEntry::MODE_READ), vfspp::InvalidOperationException);
	}
}

TEST(SevenZipFileEntryTest, OpenWrite)
{
	SevenZipFileSystem fs(TEST_RESOURCE_DIR "/7z/7zip.7z");

	IFileSystemEntry *root = fs.getRootEntry();

	{
		boost::shared_ptr<IFileSystemEntry> entry = root->getChild("test1.txt");

		ASSERT_THROW(entry->open(IFileSystemEntry::MODE_WRITE), vfspp::InvalidOperationException);
	}
	{
		ASSERT_THROW(root->open(IFileSystemEntry::MODE_WRITE), vfspp::InvalidOperationException);
	}
}
