#pragma once

#include <VFSPP/core.hpp>

namespace vfspp
{
	namespace test
	{
		bool vectorContainsEntry(const std::vector<boost::shared_ptr<IFileSystemEntry> >& vector, const std::string& path, EntryType type);
	}
}
