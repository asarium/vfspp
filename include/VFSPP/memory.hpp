
#pragma once

#include <VFSPP/core.hpp>

#include <boost/unordered_map.hpp>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

namespace vfspp
{
	namespace memory
	{
		class MemoryFileSystem;

		class VFSPP_EXPORT MemoryFileEntry : public IFileSystemEntry
		{
		private:
			EntryType type;

			std::vector<boost::shared_ptr<MemoryFileEntry> > fileEntries;
			boost::unordered_map<string_type, size_t> indexMapping;

			size_t dataSize;
			boost::shared_array<char> data;

			time_t writeTime;

			MemoryFileEntry(const string_type& path) :
				IFileSystemEntry(path), dataSize(0), writeTime(0), type(UNKNOWN) {}

			FileEntryPointer getChildInternal(const string_type& path);

		public:
			virtual ~MemoryFileEntry() {}

			virtual FileEntryPointer getChild(const string_type& path) VFSPP_OVERRIDE;

			virtual size_t numChildren() VFSPP_OVERRIDE;

			virtual void listChildren(std::vector<FileEntryPointer>& outVector) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) VFSPP_OVERRIDE;

			virtual EntryType getType() const VFSPP_OVERRIDE;

			virtual bool deleteChild(const string_type& name) VFSPP_OVERRIDE;

			virtual FileEntryPointer createEntry(EntryType type, const string_type& name) VFSPP_OVERRIDE;

			virtual void rename(const string_type& newPath) VFSPP_OVERRIDE;

			virtual time_t lastWriteTime() VFSPP_OVERRIDE { return writeTime; }

			boost::shared_ptr<MemoryFileEntry> addChild(const string_type& name, EntryType type,
				time_t write_time = 0, void* data = 0, size_t dataSize = 0);

			friend class MemoryFileSystem;
		};

		class VFSPP_EXPORT MemoryFileSystem : public IFileSystem
		{
		private:
			boost::scoped_ptr<MemoryFileEntry> rootEntry;

		public:
			MemoryFileSystem();

			virtual ~MemoryFileSystem() {}
			
			virtual MemoryFileEntry* getRootEntry() VFSPP_OVERRIDE { return rootEntry.get(); }

			virtual int supportedOperations() const VFSPP_OVERRIDE { return OP_READ; }
		};
	}
}
