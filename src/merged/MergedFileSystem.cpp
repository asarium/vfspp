
#include <VFSPP/merged.hpp>

#include <boost/foreach.hpp>

using namespace vfspp;
using namespace vfspp::merged;

using namespace boost;

MergedFileSystem::MergedFileSystem()
{
	rootEntry.reset(new MergedEntry(this, shared_ptr<IFileSystemEntry>()));
}

void MergedFileSystem::addFileSystem(IFileSystem* fileSystem)
{
	if (fileSystem == NULL)
	{
		throw InvalidOperationException("File system pointer is null!");
	}

	fileSystems.push_back(shared_ptr<IFileSystem>(fileSystem));
}

MergedEntry* MergedFileSystem::getRootEntry()
{
	return rootEntry.get();
}

int MergedFileSystem::supportedOperations() const
{
	int ops = 0;

	BOOST_FOREACH(const shared_ptr<IFileSystem>& ptr, fileSystems)
	{
		ops |= ptr->supportedOperations();
	}

	return ops;
}

void MergedFileSystem::populateChildren(MergedEntry* entry, int level)
{
	if (level <= 0)
	{
		return;
	}

	entry->cacheChildren();

	BOOST_FOREACH(shared_ptr<MergedEntry> childEntry, entry->cachedChildEntries)
	{
		if (childEntry->getType() == DIRECTORY)
		{
			populateChildren(childEntry.get(), level - 1);
		}
	}
}

void MergedFileSystem::populateEntries(int levels)
{
	populateChildren(rootEntry.get(), levels);
}
