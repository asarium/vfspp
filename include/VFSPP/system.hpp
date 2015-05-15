#pragma once

#include <boost/filesystem.hpp>

#include "vfspp_export.h"

#include "vfspp_compiler_detection.h"
#include "VFSPP/core.hpp"

namespace vfspp {
	namespace system {
		class PhysicalFileSystem;

		class VFSPP_EXPORT PhysicalEntry : public IFileSystemEntry
		{
		protected:
			PhysicalFileSystem* parentSystem;

			boost::filesystem::path entryPath;

		public:
			PhysicalEntry(PhysicalFileSystem* parentSystem, const vfspp::string_type& path);

			virtual ~PhysicalEntry() {}

			const boost::filesystem::path& getEntryPath() const { return entryPath; }

			virtual FileEntryPointer getChild(const string_type& path) VFSPP_OVERRIDE;

			virtual size_t numChildren() VFSPP_OVERRIDE;

			virtual void listChildren(std::vector<FileEntryPointer>& outVector) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) VFSPP_OVERRIDE;

			virtual EntryType getType() const VFSPP_OVERRIDE;

			virtual bool deleteChild(const string_type& name) VFSPP_OVERRIDE;

			virtual FileEntryPointer createEntry(EntryType type, const string_type& name) VFSPP_OVERRIDE;

			virtual void rename(const string_type& newPath) VFSPP_OVERRIDE;

			virtual time_t lastWriteTime() VFSPP_OVERRIDE;

			friend class PhysicalFileSystem;
		};

		class VFSPP_EXPORT PhysicalFileSystem : public IFileSystem
		{
		private:
			boost::filesystem::path physicalRoot;

			boost::scoped_ptr<PhysicalEntry> rootDir;

			int operations;

		public:
			PhysicalFileSystem(const boost::filesystem::path& physicalRoot);

			virtual ~PhysicalFileSystem() {}

			const boost::filesystem::path& getPhysicalRoot() const
			{
				return physicalRoot;
			}

			virtual PhysicalEntry* getRootEntry() VFSPP_OVERRIDE;

			void setAllowedOperations(int ops);

			virtual int supportedOperations() const VFSPP_OVERRIDE
			{
				return operations;
			}

			virtual string_type getName() const { return physicalRoot.string(); }
		};
	}
}
