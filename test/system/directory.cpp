
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/smart_ptr.hpp>

#include <VFSPP/system.hpp>
#include "globals.hpp"

#include "gtest/gtest.h"

using namespace vfspp;
using namespace vfspp::system;
using namespace vfspp::test;

using namespace boost;


TEST(PhysicalEntryTest, NumChildren)
{
	PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

	{
		ASSERT_EQ(7, fs.getRootEntry()->numChildren());
	}
	{
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.numChildren(), vfspp::InvalidOperationException);
	}
	{
		fs.setAllowedOperations(0);

		ASSERT_THROW(fs.getRootEntry()->numChildren(), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, ListChildren)
{
	PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

	std::vector<shared_ptr<IFileSystemEntry>> children;
	{
		IFileSystemEntry* rootDir = fs.getRootEntry();
		rootDir->listChildren(children);

		ASSERT_EQ(7, children.size());

		ASSERT_TRUE(vectorContainsEntry(children, "test1", DIRECTORY));
		ASSERT_TRUE(vectorContainsEntry(children, "test2", DIRECTORY));
		ASSERT_TRUE(vectorContainsEntry(children, "test3", DIRECTORY));

		ASSERT_TRUE(vectorContainsEntry(children, "test1.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test2.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test3.txt", vfspp::FILE));
		ASSERT_TRUE(vectorContainsEntry(children, "test4.txt", vfspp::FILE));
	}
	{
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.listChildren(children), vfspp::InvalidOperationException);
	}
	{
		fs.setAllowedOperations(0);

		ASSERT_THROW(fs.getRootEntry()->listChildren(children), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, GetChild)
{
	PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

	{
		IFileSystemEntry* rootDir = fs.getRootEntry();

		shared_ptr<IFileSystemEntry> child = rootDir->getChild("test1/test1.txt");

		ASSERT_TRUE(child->getPath() == "test1/test1.txt");
		ASSERT_TRUE(child->getType() == vfspp::FILE);
	}
	{
		IFileSystemEntry* rootDir = fs.getRootEntry();

		shared_ptr<IFileSystemEntry> child = rootDir->getChild("foo/bar");

		ASSERT_FALSE(child);
	}
	{
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.getChild("bar.txt"), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, DirectoryGetType)
{
	PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");

	IFileSystemEntry* rootDir = fs.getRootEntry();

	ASSERT_TRUE(rootDir->getType() == DIRECTORY);
}

TEST(PhysicalEntryTest, DeleteChild)
{
	using namespace boost::filesystem;

	{
		path testPath = path(TEST_WRITE_DIR "/system/test1");
		path testFile = testPath / "test1.txt";

		create_directories(testPath);

		std::ofstream out(testFile.native());
		out << "Test" << std::endl;
		out.close();

		EXPECT_TRUE(exists(testPath));
		EXPECT_TRUE(exists(testFile));

		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");

		IFileSystemEntry* rootDir = fs.getRootEntry();

		ASSERT_TRUE(rootDir->deleteChild("test1"));

		ASSERT_FALSE(exists(testPath));
		ASSERT_FALSE(exists(testFile));
	}
	{
		path testPath = path(TEST_WRITE_DIR "/system");
		path testFile = testPath / "test1.txt";

		create_directories(testPath);

		std::ofstream out(testFile.native());
		out << "Test" << std::endl;
		out.close();

		EXPECT_TRUE(exists(testPath));
		EXPECT_TRUE(exists(testFile));

		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");

		IFileSystemEntry* rootDir = fs.getRootEntry();

		ASSERT_TRUE(rootDir->deleteChild("test1.txt"));

		ASSERT_TRUE(exists(testPath));
		ASSERT_FALSE(exists(testFile));
	}

	{
		path testFile = path(TEST_WRITE_DIR "/system/test1.txt");

		EXPECT_FALSE(exists(testFile));

		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");

		IFileSystemEntry* rootDir = fs.getRootEntry();

		ASSERT_FALSE(rootDir->deleteChild("test1.txt"));
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.deleteChild("bar.txt"), vfspp::InvalidOperationException);
	}

	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");
		fs.setAllowedOperations(0);

		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.deleteChild("bar.txt"), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, CreateFile)
{
	using namespace boost::filesystem;

	{
		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");

		PhysicalEntry* rootDir = fs.getRootEntry();

		boost::shared_ptr<IFileSystemEntry> fileEntry = rootDir->createEntry(vfspp::FILE, "test1.txt");

		ASSERT_TRUE(exists(path(TEST_WRITE_DIR "/system/test1.txt")));
		ASSERT_TRUE(fileEntry->getType() == vfspp::FILE);

		remove(path(TEST_WRITE_DIR "/system/test1.txt"));
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.createEntry(vfspp::FILE, "bar.txt"), vfspp::InvalidOperationException);
	}
	{
		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");
		fs.setAllowedOperations(0);

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->createEntry(vfspp::FILE, "test1.txt"), vfspp::InvalidOperationException);
	}
}

TEST(PhysicalEntryTest, CreateDirectory)
{
	using namespace boost::filesystem;

	{
		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");

		PhysicalEntry* rootDir = fs.getRootEntry();

		boost::shared_ptr<IFileSystemEntry> fileEntry = rootDir->createEntry(vfspp::DIRECTORY, "test1");

		ASSERT_TRUE(exists(path(TEST_WRITE_DIR "/system/test1")));
		ASSERT_TRUE(fileEntry->getType() == DIRECTORY);

		remove(path(TEST_WRITE_DIR "/system/test1"));
	}
	{
		PhysicalFileSystem fs(TEST_RESOURCE_DIR "/system");
		PhysicalEntry entry(&fs, "test1.txt");

		ASSERT_THROW(entry.createEntry(vfspp::DIRECTORY, "bar.txt"), vfspp::InvalidOperationException);
	}
	{
		PhysicalFileSystem fs(TEST_WRITE_DIR "/system");
		fs.setAllowedOperations(0);

		PhysicalEntry* rootDir = fs.getRootEntry();

		ASSERT_THROW(rootDir->createEntry(vfspp::DIRECTORY, "test1.txt"), vfspp::InvalidOperationException);
	}
}
