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
			MergedFileSystem* parentSystem;

			boost::shared_ptr<IFileSystemEntry> containedEntry;

			std::vector<boost::shared_ptr<MergedEntry> > cachedChildEntries;

			boost::unordered_map<string_type, boost::shared_ptr<MergedEntry> > cachedChildMapping;

			bool dirty;

			void cacheChildren();

			boost::shared_ptr<MergedEntry> getEntryInternal(const string_type& path);

			void addChildren(IFileSystemEntry* entry);

		public:
			MergedEntry(MergedFileSystem* parentSystem, boost::shared_ptr<IFileSystemEntry> mergedEntry);

			virtual ~MergedEntry() {}

			virtual boost::shared_ptr<IFileSystemEntry> getChild(const string_type& path) VFSPP_OVERRIDE;

			virtual size_t numChildren() VFSPP_OVERRIDE;

			virtual void listChildren(std::vector<boost::shared_ptr<IFileSystemEntry> >& outVector) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) VFSPP_OVERRIDE;

			virtual EntryType getType() const VFSPP_OVERRIDE;

			virtual bool deleteChild(const string_type& name) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<IFileSystemEntry> createEntry(EntryType type, const string_type& name) VFSPP_OVERRIDE;

			friend class MergedFileSystem;
		};

		class VFSPP_EXPORT MergedFileSystem : public IFileSystem
		{
		private:
			std::vector<boost::shared_ptr<IFileSystem> > fileSystems;

			boost::scoped_ptr<MergedEntry> rootEntry;

			void populateChildren(MergedEntry* entry, int levels);

		public:
			MergedFileSystem();

			virtual ~MergedFileSystem() {}

			void addFileSystem(IFileSystem* fileSystem);

			virtual MergedEntry* getRootEntry() VFSPP_OVERRIDE;

			virtual int supportedOperations() const VFSPP_OVERRIDE;

			void populateEntries(int levels = 2);

			friend class MergedEntry;
		};
	}
}
