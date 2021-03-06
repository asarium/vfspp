
#include <algorithm>

#include <boost/filesystem/fstream.hpp>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "VFSPP/system.hpp"

namespace
{
	boost::filesystem::path uncomplete(const boost::filesystem::path& base, const boost::filesystem::path& path)
	{
		if (path.has_root_path())
		{
			if (path.root_path() != base.root_path())
			{
				return path;
			}
			else
			{
				return uncomplete(base.relative_path(), path.relative_path());
			}
		}
		else
		{
			if (base.has_root_path())
			{
				throw vfspp::InvalidOperationException("Cannot uncomplete a path relative path from a rooted base");
			}
			else
			{
				typedef boost::filesystem::path::const_iterator path_iterator;

				path_iterator path_it = path.begin();
				path_iterator base_it = base.begin();

				while (path_it != path.end() && base_it != base.end())
				{
					if (*path_it != *base_it) break;
					++path_it; ++base_it;
				}

				boost::filesystem::path result;
				for (; base_it != base.end(); ++base_it)
				{
					result /= "..";
				}

				for (; path_it != path.end(); ++path_it)
				{
					result /= *path_it;
				}

				return result;
			}
		}
	}
}

using namespace vfspp;
using namespace vfspp::system;

using namespace boost;
using namespace boost::filesystem;

PhysicalEntry::PhysicalEntry(PhysicalFileSystem* parentSystemIn, const vfspp::string_type& pathIn)
: IFileSystemEntry(pathIn), parentSystem(parentSystemIn)
{
	entryPath = parentSystem->getPhysicalRoot() / this->path;
}

size_t PhysicalEntry::numChildren()
{
	if ((parentSystem->supportedOperations() & OP_READ) == 0)
	{
		throw InvalidOperationException("System does not support reading!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	return std::distance(directory_iterator(entryPath), directory_iterator());
}

FileEntryPointer PhysicalEntry::getChild(const string_type& path)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	boost::filesystem::path childPath = entryPath / path;

	if (exists(childPath))
	{
		return FileEntryPointer(new PhysicalEntry(parentSystem, path));
	}
	else
	{
		return FileEntryPointer();
	}
}

void PhysicalEntry::listChildren(std::vector<FileEntryPointer>& outVector)
{
	if ((parentSystem->supportedOperations() & OP_READ) == 0)
	{
		throw InvalidOperationException("System does not support reading!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	outVector.clear();

	directory_iterator end;

	for (directory_iterator iter(entryPath); iter != end; ++iter)
	{
		boost::filesystem::path relativ = uncomplete(parentSystem->getPhysicalRoot(), iter->path());

		outVector.push_back(FileEntryPointer(new PhysicalEntry(parentSystem, relativ.generic_string())));
	}
}

EntryType PhysicalEntry::getType() const
{
	if (!exists(entryPath))
	{
		return UNKNOWN;
	}
	else if (is_directory(entryPath))
	{
		return DIRECTORY;
	}
	else
	{
		return FILE;
	}
}

bool PhysicalEntry::deleteChild(const string_type& name)
{
	if ((parentSystem->supportedOperations() & OP_DELETE) == 0)
	{
		throw InvalidOperationException("System does not support deleting!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	boost::filesystem::path childPath = entryPath / name;

	if (!exists(childPath))
	{
		return false;
	}

	if (is_directory(childPath))
	{
		boost::system::error_code errorCode;

		uintmax_t removed = remove_all(childPath, errorCode);

		if (errorCode)
		{
			throw FileSystemException(errorCode.message());
		}
		else
		{
			return removed > 0;
		}
	}
	else
	{
		boost::system::error_code errorCode;

		bool success = remove(childPath, errorCode);

		if (errorCode)
		{
			throw FileSystemException(errorCode.message());
		}
		else
		{
			return success;
		}
	}
}

FileEntryPointer PhysicalEntry::createEntry(EntryType type, const string_type& name)
{
	if ((parentSystem->supportedOperations() & OP_CREATE) == 0)
	{
		throw InvalidOperationException("System does not support creating!");
	}

	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	if (!(type == DIRECTORY || type == FILE))
	{
		throw InvalidOperationException("Invalid entry type to create specified!");
	}

	filesystem::path createPath = entryPath / name;

	if (!exists(createPath))
	{
		switch (type)
		{
		case FILE:
		{
			filesystem::path parent = createPath.parent_path();
			if (!is_directory(parent))
			{
				create_directories(parent);
			}

			// This creates the file
			boost::filesystem::ofstream newFile(createPath);
			newFile.close();
			break;
		}
		case DIRECTORY:
		{
			create_directories(createPath);
			break;
		}
		default:
			throw InvalidOperationException("Invalid type specified!");
		}
	}
	else
	{
		switch (type)
		{
		case FILE:
			if (!is_regular_file(createPath))
			{
				throw InvalidOperationException("Path exists but is no file!");
			}
			break;
		case DIRECTORY:
			if (!is_directory(createPath))
			{
				throw InvalidOperationException("Path exists but is no directory!");
			}
		default:
			throw InvalidOperationException("Invalid type specified!");
		}
	}

	return FileEntryPointer(new PhysicalEntry(parentSystem, name));
}

boost::shared_ptr<std::streambuf> PhysicalEntry::open(int mode)
{
	using namespace boost::iostreams;

	if ((mode & MODE_WRITE) != 0 && (parentSystem->supportedOperations() & OP_WRITE) == 0)
	{
		throw InvalidOperationException("System does not support writing!");
	}

	if ((mode & MODE_READ) != 0 && (parentSystem->supportedOperations() & OP_READ) == 0)
	{
		throw InvalidOperationException("System does not support writing!");
	}

	if (getType() != FILE)
	{
		throw InvalidOperationException("Entry is no file!");
	}

	std::ios_base::openmode openmode = std::ios::binary;

	if (mode & MODE_WRITE)
	{
		openmode = openmode | std::ios::out;
	}
	
	if (mode & MODE_READ)
	{
		openmode = openmode | std::ios::in;
	}

	if (mode & MODE_MEMORY_MAPPED)
	{
		typedef stream_buffer<mapped_file> mapped_buffer;

		basic_mapped_file_params<boost::filesystem::path> params(getEntryPath());
		params.mode = openmode;

		shared_ptr<mapped_buffer> buffer = shared_ptr<mapped_buffer>(new mapped_buffer(params));
		if (!buffer->is_open())
		{
			throw FileSystemException("Failed to open memory mapped file!");
		}
		else
		{
			return buffer;
		}
	}
	else
	{
		boost::shared_ptr<boost::filesystem::filebuf> buffer(new boost::filesystem::filebuf());

		buffer->open(entryPath, openmode);

		if (!buffer->is_open())
		{
			throw FileSystemException("Failed to open file!");
		}
		else
		{
			return buffer;
		}
	}
}

void PhysicalEntry::rename(const string_type& newName)
{
	if (!(parentSystem->supportedOperations() & (OP_DELETE | OP_CREATE)))
	{
		throw InvalidOperationException("Operations needed for renaming are not supported!");
	}

	filesystem::path newPath(parentSystem->getPhysicalRoot() / newName);

	filesystem::rename(entryPath, newPath);
}

time_t PhysicalEntry::lastWriteTime()
{
	return last_write_time(entryPath);
}
