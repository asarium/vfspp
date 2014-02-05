
#include "globals.hpp"

#include <boost/foreach.hpp>

using namespace vfspp;

using namespace boost;

namespace vfspp
{
	namespace test
	{
		bool vectorContainsEntry(const std::vector<boost::shared_ptr<IFileSystemEntry> >& vector, const std::string& path, EntryType type)
		{
			BOOST_FOREACH(shared_ptr<IFileSystemEntry> entry, vector)
			{
				if (entry->getPath() == path && entry->getType() == type)
				{
					return true;
				}
			}

			return false;
		}
	}
}