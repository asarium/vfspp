
#include "VFSPP/core.hpp"
#include "VFSPP/util.hpp"

namespace vfspp
{
	IFileSystemEntry::IFileSystemEntry(const string_type& pathIn) : path(pathIn)
	{
		this->path = util::normalizePath(this->path);
	}

	namespace util
	{
		int modeToOperation(int mode)
		{
			int out = 0;

			if (mode & IFileSystemEntry::MODE_READ)
			{
				out |= OP_READ;
			}

			if (mode & IFileSystemEntry::MODE_WRITE)
			{
				out |= OP_WRITE;
			}

			return out;
		}
	}
}
