#pragma once

#include <boost/unordered_map.hpp>

#include "VFSPP/core.hpp"

namespace vfspp
{
	namespace merged
	{
		class MergedFileSystem;

		class VFSPP_EXPORT MergedEntry : public IFileSystemEntry
		{
		private:
			FileEntryPointer containedEntry;

			std::vector<boost::shared_ptr<MergedEntry> > cachedChildEntries;

			boost::unordered_map<string_type, boost::shared_ptr<MergedEntry> > cachedChildMapping;

			bool dirty;

			void cacheChildren();

			boost::shared_ptr<MergedEntry> getEntryInternal(const string_type& path);

			void addChildren(IFileSystemEntry* entry);

		public:
			MergedEntry(MergedFileSystem* parentSystem, FileEntryPointer mergedEntry);

			virtual ~MergedEntry() {}

			FileEntryPointer getContainedEntry() const { return containedEntry; }

			virtual FileEntryPointer getChild(const string_type& path) VFSPP_OVERRIDE;

			virtual size_t numChildren() VFSPP_OVERRIDE;

			virtual void listChildren(std::vector<FileEntryPointer>& outVector) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) VFSPP_OVERRIDE;

			virtual EntryType getType() const VFSPP_OVERRIDE;

			virtual bool deleteChild(const string_type& name) VFSPP_OVERRIDE;

			virtual FileEntryPointer createEntry(EntryType type, const string_type& name) VFSPP_OVERRIDE;

			virtual void rename(const string_type& newPath) VFSPP_OVERRIDE;

			virtual time_t lastWriteTime() VFSPP_OVERRIDE;

			friend class MergedFileSystem;

			MergedFileSystem* parentSystem;
		};

		class VFSPP_EXPORT MergedFileSystem : public IFileSystem
		{
		private:
			std::vector<boost::shared_ptr<IFileSystem> > fileSystems;

			boost::scoped_ptr<MergedEntry> rootEntry;

			bool caseInsensitive;

			void populateChildren(MergedEntry* entry, int levels);

		public:
			MergedFileSystem();

			virtual ~MergedFileSystem() {}

			void addFileSystem(IFileSystem* fileSystem);

			virtual MergedEntry* getRootEntry() VFSPP_OVERRIDE;

			virtual int supportedOperations() const VFSPP_OVERRIDE;

			void populateEntries(int levels = 2);

			void setCaseInsensitive(bool b) { caseInsensitive = b; }

			friend class MergedEntry;
		};
	}
}
