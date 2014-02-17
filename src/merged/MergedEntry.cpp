
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "VFSPP/merged.hpp"
#include "VFSPP/util.hpp"

using namespace vfspp;
using namespace vfspp::merged;

using namespace boost;

namespace
{
	boost::shared_ptr<MergedEntry> getParent(MergedEntry* entry)
	{
		size_t slash = entry->getPath().find_last_of(DirectorySeparatorChar);

		if (slash != string_type::npos)
		{
			string_type parentPath = entry->getPath();
			parentPath.resize(slash);

			return static_pointer_cast<MergedEntry>(entry->parentSystem->getRootEntry()->getChild(parentPath));
		}
		else
		{
			// return null for root
			return shared_ptr<MergedEntry>();
		}
	}
}

typedef unordered_map<string_type, shared_ptr<MergedEntry> >::iterator ChildMapping;

MergedEntry::MergedEntry(MergedFileSystem* parentSystem, FileEntryPointer contained) :
IFileSystemEntry(contained ? contained->getPath() : ""), parentSystem(parentSystem), containedEntry(contained),
dirty(true)
{
}

void MergedEntry::addChildren(IFileSystemEntry* entry)
{
	if (entry->getType() != DIRECTORY)
	{
		return;
	}

	std::vector<FileEntryPointer> entries;

	entry->listChildren(entries);

	BOOST_FOREACH(FileEntryPointer& childEntry, entries)
	{
		string_type entryName = util::normalizePath(childEntry->getPath(), parentSystem->caseInsensitive);
		if (!isRoot())
		{
			entryName = util::normalizePath(entryName.substr(path.size()));
		}
		size_t slash = entryName.find_first_of(DirectorySeparatorChar);

		if (slash != string_type::npos)
		{
			entryName.resize(slash);
		}

		if (cachedChildMapping.find(entryName) == cachedChildMapping.end())
		{
			// This entry hasn't been found yet
			shared_ptr<MergedEntry> newEntry(new MergedEntry(parentSystem, childEntry));

			cachedChildMapping.insert(std::make_pair(entryName, newEntry));
			cachedChildEntries.push_back(newEntry);
		}
	}
}

void MergedEntry::cacheChildren()
{
	cachedChildEntries.clear();
	cachedChildMapping.clear();

	BOOST_FOREACH(shared_ptr<IFileSystem>& system, parentSystem->fileSystems)
	{
		if (system->supportedOperations() & OP_READ)
		{
			if (isRoot())
			{
				addChildren(system->getRootEntry());
			}
			else
			{
				FileEntryPointer entry = system->getRootEntry()->getChild(path);

				if (entry)
				{
					addChildren(entry.get());
				}
			}
		}
	}

	dirty = false;
}

boost::shared_ptr<MergedEntry> MergedEntry::getEntryInternal(const string_type& path)
{
	if (dirty)
	{
		cacheChildren();
	}

	size_t separator = path.find_first_of(DirectorySeparatorChar);

	if (separator == string_type::npos)
	{
		ChildMapping found = cachedChildMapping.find(path);

		if (found != cachedChildMapping.end())
		{
			return found->second;
		}
	}
	else
	{
		string_type thisLevel = path.substr(0, separator);

		ChildMapping found = cachedChildMapping.find(thisLevel);

		if (found != cachedChildMapping.end())
		{
			return found->second->getEntryInternal(path.substr(separator + 1));
		}
	}

	return shared_ptr<MergedEntry>();
}

FileEntryPointer MergedEntry::getChild(const string_type& path)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	return getEntryInternal(util::normalizePath(path, parentSystem->caseInsensitive));
}

size_t MergedEntry::numChildren()
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	if (dirty)
	{
		cacheChildren();
	}

	return cachedChildEntries.size();
}

void MergedEntry::listChildren(std::vector<FileEntryPointer>& outVector)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	if (dirty)
	{
		cacheChildren();
	}

	outVector.clear();

	std::copy(cachedChildEntries.begin(), cachedChildEntries.end(), std::back_inserter(outVector));
}

boost::shared_ptr<std::streambuf> MergedEntry::open(int mode)
{
	int ops = util::modeToOperation(mode);

	if (!(parentSystem->supportedOperations() & ops))
	{
		throw InvalidOperationException("No filesystem supports the requested operations!");
	}

	if (getType() != FILE)
	{
		throw InvalidOperationException("Entry is no file!");
	}

	try
	{
		// First try to open the contained entry
		return containedEntry->open(mode);
	}
	catch (...)
	{
		// If that fails try to open a file of another filesystem
	}

	BOOST_FOREACH(shared_ptr<IFileSystem>& system, parentSystem->fileSystems)
	{
		if (system->supportedOperations() & ops)
		{
			shared_ptr<IFileSystemEntry> entry = system->getRootEntry()->getChild(path);

			if (entry && entry->getType() == FILE)
			{
				try
				{
					shared_ptr<std::streambuf> buffer = entry->open(mode);

					if (buffer)
					{
						// Stop if we have sucessfully opened a file
						return buffer;
					}
				}
				catch (const FileSystemException&)
				{
					// Ignore filesystem errors and continue searching
				}
			}
		}
	}

	throw FileSystemException("Failed to open file from any filesystem!");
}

EntryType MergedEntry::getType() const
{
	if (!isRoot())
	{
		return containedEntry->getType();
	}
	else
	{
		// Root directory
		return DIRECTORY;
	}
}

bool MergedEntry::deleteChild(const string_type& name)
{
	if (!(parentSystem->supportedOperations() & OP_DELETE))
	{
		throw InvalidOperationException("No filesystem supports deleting!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	string_type normalized = util::normalizePath(name);

	// we want to delete the entry in the directory that actually contains it to keep
	// recaching overhead as small as possible
	size_t slash = normalized.find(DirectorySeparatorStr);
	if (slash != string_type::npos)
	{
		// remove everything after the slash
		normalized.resize(slash);

		FileEntryPointer entry = getEntryInternal(normalized);

		if (entry)
		{
			return entry->deleteChild(name.substr(slash + 1, name.size() - 1));
		}
		else
		{
			return false;
		}
	}

	bool success = false;
	BOOST_FOREACH(shared_ptr<IFileSystem>& system, parentSystem->fileSystems)
	{
		if (system->supportedOperations() & OP_DELETE)
		{
			FileEntryPointer entry = system->getRootEntry()->getChild(path);

			if (entry && entry->getType() == DIRECTORY)
			{
				try
				{
					success = entry->deleteChild(name) || success;
				}
				catch (const FileSystemException&)
				{
					// Ignore filesystem errors and continue searching
				}
			}
		}
	}

	return success;
}

FileEntryPointer MergedEntry::createEntry(EntryType type, const string_type& name)
{
	if (!(parentSystem->supportedOperations() & OP_CREATE))
	{
		throw InvalidOperationException("No filesystem supports deleting!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	BOOST_FOREACH(shared_ptr<IFileSystem>& system, parentSystem->fileSystems)
	{
		if (system->supportedOperations() & OP_CREATE)
		{
			FileEntryPointer entry = system->getRootEntry()->getChild(path);

			if (entry && entry->getType() == DIRECTORY)
			{
				try
				{
					FileEntryPointer newEntry = entry->createEntry(type, name);

					if (newEntry)
					{
						// Stop if we have sucessfully created an entry
						return shared_ptr<MergedEntry>(new MergedEntry(parentSystem, newEntry));
					}
				}
				catch (const FileSystemException&)
				{
					// Ignore filesystem errors and continue searching
				}
			}
		}
	}

	return FileEntryPointer();
}

void MergedEntry::rename(const string_type& newPath)
{
	if (isRoot())
	{
		throw FileSystemException("Cannot rename root!");
	}

	containedEntry->rename(newPath);

	shared_ptr<MergedEntry> parent = getParent(this);

	if (parent)
	{
		parent->dirty = true;
	}
	else
	{
		parentSystem->getRootEntry()->dirty = true;
	}
}

time_t MergedEntry::lastWriteTime()
{
	return containedEntry->lastWriteTime();
}
