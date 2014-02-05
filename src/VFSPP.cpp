
#include <boost/algorithm/string.hpp>

#include "VFSPP/core.hpp"

namespace vfspp
{
	IFileSystemEntry::IFileSystemEntry(const string_type& pathIn) : path(pathIn)
	{
		this->path = normalizePath(this->path);
	}

	string_type normalizePath(const string_type& inPath)
	{
		string_type outPath(inPath);

		boost::trim(outPath);

		boost::trim_if(outPath, boost::is_any_of(DirectorySeparatorStr));

		return outPath;
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
