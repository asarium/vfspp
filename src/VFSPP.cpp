
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

		string_type normalizePath(const char* inPath)
		{
			string_type outPath(inPath);

			boost::trim(outPath);

			boost::trim_if(outPath, boost::is_any_of(DirectorySeparatorStr));

			return outPath;
		}
	}
}
