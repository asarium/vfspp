
#include "VFSPP/core.hpp"

namespace vfspp
{
	IFileSystemEntry::IFileSystemEntry(const string_type& pathIn) : path(pathIn)
	{
		this->path = normalizePath(this->path);
	}

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
