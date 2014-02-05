#pragma once

#include <vfspp_export.h>

#include <VFSPP/defines.hpp>
#include <VFSPP/core.hpp>

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

extern "C"
{
#include <7zFile.h>
#include <7z.h>
}

namespace vfspp {
	namespace sevenzip {
		class SevenZipFileSystem;

		class VFSPP_EXPORT SevenZipFileEntry : public IFileSystemEntry
		{
		private:
			SevenZipFileSystem *parentSystem;
			boost::filesystem::path entryPath;

			EntryType getEntryType(const string_type& path) const;

		public:
			SevenZipFileEntry(SevenZipFileSystem* parentSystem, const string_type& path);

			virtual ~SevenZipFileEntry() {}

			virtual boost::shared_ptr<IFileSystemEntry> getChild(const string_type& path) VFSPP_OVERRIDE;

			virtual size_t numChildren() VFSPP_OVERRIDE;

			virtual void listChildren(std::vector<boost::shared_ptr<IFileSystemEntry> >& outVector) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) VFSPP_OVERRIDE;

			virtual EntryType getType() const VFSPP_OVERRIDE;

			virtual bool deleteChild(const string_type& name) VFSPP_OVERRIDE;

			virtual boost::shared_ptr<IFileSystemEntry> createEntry(EntryType type, const string_type& name) VFSPP_OVERRIDE;
		};

		class VFSPP_EXPORT SevenZipFileSystem : public IFileSystem
		{
		private:
			struct FileData
			{
				string_type name;
				size_t index;
				UInt64 size;
				UInt32 crc;

				UInt64 unpackedSize;
				UInt64 packedSize;

				EntryType type;
			};

			std::vector<FileData> fileData;
			boost::unordered_map<string_type, int> fileIndexes;

			string_type filePath;

			// Extract variables
			UInt32 blockIndex;
			Byte* outBuffer;
			size_t outBufferSize;

			// 7-zip variables
			CFileInStream archiveStream;
			CSzArEx db;
			CLookToRead lookStream;
			ISzAlloc allocImp;
			ISzAlloc allocTempImp;

			// Temporary buffers
			UInt16 *tempBuf;
			size_t tempBufSize;

			boost::scoped_ptr<SevenZipFileEntry> rootEntry;

			int GetFileName(const CSzArEx* db, int i);

			boost::shared_array<char> extractEntry(const string_type& path, size_t& arraySize);

			friend class SevenZipFileEntry;

		public:
			SevenZipFileSystem(const string_type& filePath);

			virtual ~SevenZipFileSystem();

			virtual SevenZipFileEntry* getRootEntry() VFSPP_OVERRIDE;

			virtual int supportedOperations() const VFSPP_OVERRIDE;
		};
	}
}
