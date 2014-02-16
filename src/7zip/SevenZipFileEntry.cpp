#include "VFSPP/7zip.hpp"

extern "C"
{
#include <Types.h>
#include <7zAlloc.h>
#include <7zCrc.h>
}

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/array.hpp>

using namespace vfspp;
using namespace vfspp::sevenzip;

using namespace boost;

namespace
{
	template<typename Ch>
	class MemoryBuffer : public boost::iostreams::basic_array<Ch>
	{
	private:
		// We keep this here so the data is deallocated when this object is deleted
		boost::shared_array<Ch> dataPtr;

	public:
		MemoryBuffer(shared_array<Ch> data, size_t n) : dataPtr(data), boost::iostreams::basic_array(data.get(), n)
		{
		}
	};
}

SevenZipFileEntry::SevenZipFileEntry(SevenZipFileSystem* parentSystem, const string_type& path)
: IFileSystemEntry(path), parentSystem(parentSystem), entryPath(path)
{
}

FileEntryPointer SevenZipFileEntry::getChild(const string_type& path)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	string_type childPath = (entryPath / path).generic_string();

	EntryType type = getEntryType(childPath);

	if (type == UNKNOWN)
	{
		return FileEntryPointer();
	}
	else
	{
		return FileEntryPointer(new SevenZipFileEntry(parentSystem, childPath));
	}
}

size_t SevenZipFileEntry::numChildren()
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	size_t num = 0;
	BOOST_FOREACH(const SevenZipFileSystem::FileData& data, parentSystem->fileData)
	{
		if (algorithm::starts_with(data.name, path))
		{
			// This is a child of this directory, now we need to check if it's in a subdirectory
			size_t pos = data.name.find_last_of('/');

			if (pos == string_type::npos)
			{
				// Couldn't find separator, it's a child if we are the root
				if (path.length() == 0)
				{
					++num;
				}
			}
			else
			{
				if (pos == path.length())
				{
					++num;
				}
			}
		}
	}

	return num;
}

void SevenZipFileEntry::listChildren(std::vector<FileEntryPointer>& outVector)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	outVector.clear();

	BOOST_FOREACH(const SevenZipFileSystem::FileData& data, parentSystem->fileData)
	{
		if (algorithm::starts_with(data.name, path))
		{
			// This is a child of this directory, now we need to check if it's in a subdirectory
			size_t pos = data.name.find_last_of('/');

			if (pos == string_type::npos)
			{
				// Couldn't find separator, it's a child if we are the root
				if (path.length() == 0)
				{
					outVector.push_back(FileEntryPointer(new SevenZipFileEntry(parentSystem, data.name)));
				}
			}
			else
			{
				if (pos == path.length())
				{
					outVector.push_back(FileEntryPointer(new SevenZipFileEntry(parentSystem, data.name)));
				}
			}
		}
	}
}

EntryType SevenZipFileEntry::getType() const
{
	return getEntryType(entryPath.generic_string());
}

EntryType SevenZipFileEntry::getEntryType(const string_type& path) const
{
	if (path.length() == 0)
	{
		// Special case: The root entry is always a directory
		return DIRECTORY;
	}

	boost::unordered_map<string_type, int>::const_iterator iter = parentSystem->fileIndexes.find(path);

	if (iter == parentSystem->fileIndexes.end())
	{
		return UNKNOWN;
	}
	else
	{
		return parentSystem->fileData[iter->second].type;
	}
}

bool SevenZipFileEntry::deleteChild(const string_type& name)
{
	throw InvalidOperationException("7-zip archives are read only!");
}

FileEntryPointer SevenZipFileEntry::createEntry(EntryType type, const string_type& name)
{
	throw InvalidOperationException("7-zip archives are read only!");
}

boost::shared_ptr<std::streambuf> SevenZipFileEntry::open(int mode)
{
	if (getType() != FILE)
	{
		throw InvalidOperationException("Entry is no file!");
	}

	if (mode & MODE_WRITE)
	{
		throw InvalidOperationException("7-zip archives are read only!");
	}

	if (mode & MODE_MEMORY_MAPPED)
	{
		throw FileSystemException("7-zip entries can't be memory mapped!");
	}

	size_t size;
	shared_array<char> data = parentSystem->extractEntry(path, size);

	return shared_ptr<std::streambuf>(new boost::iostreams::stream_buffer<MemoryBuffer<char>>(MemoryBuffer<char>(data, size)));
}

void SevenZipFileEntry::rename(const string_type& newPath)
{
	throw InvalidOperationException("7-zip archives are read-only!");
}

time_t SevenZipFileEntry::lastWriteTime()
{
	boost::unordered_map<string_type, int>::const_iterator iter = parentSystem->fileIndexes.find(path);

	if (iter == parentSystem->fileIndexes.end())
	{
		return 0;
	}
	else
	{
		return parentSystem->fileData[iter->second].write_time;
	}
}
