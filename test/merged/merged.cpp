
#include <VFSPP/merged.hpp>
#include <VFSPP/system.hpp>
#include <VFSPP/7zip.hpp>

#include <globals.hpp>

#include <boost/foreach.hpp>

#include "gtest/gtest.h"

using namespace vfspp;

using namespace vfspp::merged;
using namespace vfspp::system;
using namespace vfspp::system;
using namespace vfspp::sevenzip;

using namespace vfspp::test;

using namespace boost;

namespace
{
	void printChildrenRec(IFileSystemEntry* entry)
	{
		if (entry->getType() == DIRECTORY)
		{
			std::cout << std::setw(10) << "DIRECTORY" << "  ";
		}
		else
		{
			std::cout << std::setw(10) << "FILE" << "  ";
		}

		std::cout << entry->getPath() << std::endl;

		if (entry->getType() == DIRECTORY)
		{
			std::vector<shared_ptr<IFileSystemEntry> > children;

			entry->listChildren(children);

			BOOST_FOREACH(shared_ptr<IFileSystemEntry> child, children)
			{
				printChildrenRec(child.get());
			}
		}
	}

	class MergedEntryTest : public ::testing::Test
	{
	public:
		MergedFileSystem fileSystem;

		void SetUp()
		{
			PhysicalFileSystem* writeSystem = new PhysicalFileSystem(TEST_WRITE_DIR "/system");
			writeSystem->setAllowedOperations(OP_READ | OP_WRITE | OP_CREATE | OP_DELETE);

			PhysicalFileSystem* readSystem = new PhysicalFileSystem(TEST_RESOURCE_DIR "/system");
			readSystem->setAllowedOperations(OP_READ);

			fileSystem.addFileSystem(writeSystem);
			fileSystem.addFileSystem(readSystem);
			fileSystem.addFileSystem(new SevenZipFileSystem(TEST_RESOURCE_DIR "/7z/7zip.7z"));

			fileSystem.populateEntries();
		}
	};
}

TEST_F(MergedEntryTest, NumChildren)
{
	ASSERT_EQ(8, fileSystem.getRootEntry()->numChildren());
}

TEST_F(MergedEntryTest, ListChildren)
{
	std::vector<shared_ptr<IFileSystemEntry> > children;
	fileSystem.getRootEntry()->listChildren(children);

	ASSERT_EQ(8, children.size());

	ASSERT_TRUE(vectorContainsEntry(children, "test1", DIRECTORY));
	ASSERT_TRUE(vectorContainsEntry(children, "test2", DIRECTORY));
	ASSERT_TRUE(vectorContainsEntry(children, "test3", DIRECTORY));

	ASSERT_TRUE(vectorContainsEntry(children, "test1.txt", vfspp::FILE));
	ASSERT_TRUE(vectorContainsEntry(children, "test2.txt", vfspp::FILE));
	ASSERT_TRUE(vectorContainsEntry(children, "test3.txt", vfspp::FILE));
	ASSERT_TRUE(vectorContainsEntry(children, "test4.txt", vfspp::FILE));
	ASSERT_TRUE(vectorContainsEntry(children, "test5.txt", vfspp::FILE));
}

TEST_F(MergedEntryTest, GetChild)
{
	{
		shared_ptr<IFileSystemEntry> child = fileSystem.getRootEntry()->getChild("test1.txt");

		ASSERT_STREQ("test1.txt", child->getPath().c_str());
		ASSERT_EQ(vfspp::FILE, child->getType());
	}
	{
		ASSERT_FALSE(fileSystem.getRootEntry()->getChild("foo.txt"));
	}
}

TEST_F(MergedEntryTest, OpenRead)
{
	boost::shared_ptr<std::streambuf> buffer = fileSystem.getRootEntry()->getChild("test1.txt")->open(IFileSystemEntry::MODE_READ);
	std::istream stream(buffer.get());

	std::string content;
	content.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

	ASSERT_STREQ("TestTestTest", content.c_str());
}

