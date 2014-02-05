
#include <VFSPP/system.hpp>

using namespace vfspp::system;

using namespace boost::filesystem;

PhysicalFileSystem::PhysicalFileSystem(const boost::filesystem::path& physicalRoot)
: physicalRoot(physicalRoot), operations(OP_READ | OP_WRITE | OP_DELETE | OP_CREATE)
{
	rootDir.reset(new PhysicalEntry(this, ""));
}

PhysicalEntry* PhysicalFileSystem::getRootEntry()
{
	return rootDir.get();
}

void PhysicalFileSystem::setAllowedOperations(int ops)
{
	operations = ops;
}
