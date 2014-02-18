
#include <VFSPP/memory.hpp>

namespace vfspp
{
	namespace memory
	{
		MemoryFileSystem::MemoryFileSystem()
		{
			rootEntry.reset(new MemoryFileEntry(""));
			rootEntry->type = DIRECTORY;
		}
	}
}
