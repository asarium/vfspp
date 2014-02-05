
#include <algorithm>

#include <boost/filesystem/fstream.hpp>

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

boost::shared_ptr<IFileSystemEntry> PhysicalEntry::getChild(const string_type& path)
{
	if (getType() != DIRECTORY)
	{
		throw InvalidOperationException("Entry is no directory!");
	}

	boost::filesystem::path childPath = entryPath / path;

	if (exists(childPath))
	{
		return boost::shared_ptr<IFileSystemEntry>(new PhysicalEntry(parentSystem, path));
	}
	else
	{
		return boost::shared_ptr<IFileSystemEntry>();
	}
}

void PhysicalEntry::listChildren(std::vector<boost::shared_ptr<IFileSystemEntry> >& outVector)
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

		outVector.push_back(boost::shared_ptr<IFileSystemEntry>(new PhysicalEntry(parentSystem, relativ.generic_string())));
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

boost::shared_ptr<IFileSystemEntry> PhysicalEntry::createEntry(EntryType type, const string_type& name)
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
			if (!is_regular_file(entryPath))
			{
				throw InvalidOperationException("Path exists but is no file!");
			}
			break;
		case DIRECTORY:
			if (!is_directory(entryPath))
			{
				throw InvalidOperationException("Path exists but is no directory!");
			}
		default:
			throw InvalidOperationException("Invalid type specified!");
		}
	}

	return boost::shared_ptr<IFileSystemEntry>(new PhysicalEntry(parentSystem, name));
}

boost::shared_ptr<std::streambuf> PhysicalEntry::open(int mode)
{
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

	boost::shared_ptr<boost::filesystem::filebuf> buffer(new boost::filesystem::filebuf());

	// Thank you gcc for not allowing me to assign 0 to openmode...
	std::ios_base::openmode openmode;

	if ((mode & MODE_WRITE) == MODE_WRITE)
	{
		openmode = std::ios::out;
	}
	else if ((mode & MODE_READ) == MODE_READ)
	{
		openmode = std::ios::in;
	}
	else if (mode & (MODE_READ | MODE_WRITE))
	{
		openmode = std::ios::in | std::ios::out;
	}
	else
	{
		throw InvalidOperationException("Invalid modes specified!");
	}

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
