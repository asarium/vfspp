
extern "C"
{
#include <Types.h>
#include <7zAlloc.h>
#include <7zCrc.h>
}

#include <boost/system/error_code.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <utf8.h>

#include "VFSPP/7zip.hpp"

using namespace vfspp;
using namespace vfspp::sevenzip;

namespace
{
	bool inited = false;

	void sevenzip_init()
	{
		if (inited)
		{
			return;
		}

		CrcGenerateTable();

		inited = true;
	}

	const char* GetErrorStr(int err)
	{
		switch (err) {
		case SZ_OK:
			return "OK";
		case SZ_ERROR_FAIL:
			return "Extracting failed";
		case SZ_ERROR_CRC:
			return "CRC error (archive corrupted?)";
		case SZ_ERROR_INPUT_EOF:
			return "Unexpected end of file (truncated?)";
		case SZ_ERROR_MEM:
			return "Out of memory";
		case SZ_ERROR_UNSUPPORTED:
			return "Unsupported archive";
		case SZ_ERROR_NO_ARCHIVE:
			return "Archive not found";
		}
		return "Unknown error";
	}
}

SevenZipFileSystem::SevenZipFileSystem(const string_type& path) :
	filePath(path),
	tempBuf(NULL),
	tempBufSize(0),
	blockIndex(0xFFFFFFFF),
	outBuffer(NULL),
	outBufferSize(0)
{
	if (!inited)
	{
		sevenzip_init();
	}

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	SzArEx_Init(&db);

	WRes wres = InFile_Open(&archiveStream.file, path.c_str());
	if (wres)
	{
		boost::system::error_code e(wres, boost::system::get_system_category());

		throw FileSystemException((boost::format("Failed to open: %1% (%2%)") % e.message() % e.value()).str());
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	SRes res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
	if (res != SZ_OK)
	{
		throw FileSystemException((boost::format("Error opening: %1%") % GetErrorStr(res)).str());
	}

	// In 7zip talk, folders are pack-units (solid blocks),
	// not related to file-system folders.
	UInt64* folderUnpackSizes = new UInt64[db.db.NumFolders];
	for (unsigned int fi = 0; fi < db.db.NumFolders; fi++)
	{
		folderUnpackSizes[fi] = SzFolder_GetUnpackSize(db.db.Folders + fi);
	}

	// Get contents of archive and store name->int mapping
	for (unsigned int i = 0; i < db.db.NumFiles; ++i)
	{
		CSzFileItem* f = db.db.Files + i;

		int written = GetFileName(&db, i);
		if (written <= 0) {
			// TODO: Implement logging
			continue;
		}

		string_type utf8Name;
		utf8Name.reserve(written);

		utf8::utf16to8(tempBuf, tempBuf + written, std::back_inserter(utf8Name));

		utf8Name = normalizePath(utf8Name);

		// Remove a trailing 0-char as that screws with the string
		if (utf8Name[utf8Name.size() - 1] == 0)
		{
			utf8Name.resize(utf8Name.size() - 1);
		}

		FileData fd;
		fd.name = utf8Name;
		fd.index = i;

		if (f->MTimeDefined)
		{
			// From boost, seems to works although I don't know why...
			__int64 t = (static_cast<__int64>(f->MTime.High) << 32) + f->MTime.Low;
#   if !defined(_MSC_VER) || _MSC_VER > 1300 // > VC++ 7.0
			t -= 116444736000000000LL;
#   else
			t -= 116444736000000000;
#   endif
			t /= 10000000;
			fd.write_time =  static_cast<std::time_t>(t);
		}
		else
		{
			fd.write_time = 0;
		}

		if (!f->IsDir)
		{
			fd.size = f->Size;
			fd.crc = (f->Size > 0) ? f->Crc : 0;

			const UInt32 folderIndex = db.FileIndexToFolderIndexMap[i];
			if (folderIndex == ((UInt32)-1))
			{
				// file has no folder assigned
				fd.unpackedSize = f->Size;
				fd.packedSize = f->Size;
			}
			else
			{
				fd.unpackedSize = folderUnpackSizes[folderIndex];
				fd.packedSize = db.db.PackSizes[folderIndex];
			}

			fd.type = FILE;
		}
		else
		{
			fd.size = 0;
			fd.crc = 0;
			fd.unpackedSize = 0;
			fd.packedSize = 0;

			fd.type = DIRECTORY;
		}

		fileData.push_back(fd);
		fileIndexes[utf8Name] = fileData.size() - 1;
	}

	delete[] folderUnpackSizes;

	rootEntry.reset(new SevenZipFileEntry(this, ""));
}

SevenZipFileSystem::~SevenZipFileSystem()
{
	if (outBuffer != NULL)
	{
		IAlloc_Free(&allocImp, outBuffer);
	}

	if (tempBuf != NULL)
	{
		SzFree(NULL, tempBuf);
		tempBuf = NULL;
		tempBufSize = 0;
	}

	File_Close(&archiveStream.file);

	SzArEx_Free(&db, &allocImp);
}

SevenZipFileEntry* SevenZipFileSystem::getRootEntry()
{
	return rootEntry.get();
}

int SevenZipFileSystem::supportedOperations() const
{
	return OP_READ;
}

int SevenZipFileSystem::GetFileName(const CSzArEx* db, int i)
{
	size_t len = SzArEx_GetFileNameUtf16(db, i, NULL);

	if (len > tempBufSize) {
		SzFree(NULL, tempBuf);
		tempBufSize = len;
		tempBuf = (UInt16 *)SzAlloc(NULL, tempBufSize * sizeof(tempBuf[0]));
		if (tempBuf == 0) {
			return SZ_ERROR_MEM;
		}
	}
	tempBuf[len - 1] = 0;
	return SzArEx_GetFileNameUtf16(db, i, tempBuf);
}

boost::shared_array<char> SevenZipFileSystem::extractEntry(const string_type& path, size_t& arraySize)
{
	size_t offset;
	size_t outSizeProcessed;
	SRes res;

	boost::unordered_map<string_type, int>::const_iterator iter = fileIndexes.find(path);

	if (iter == fileIndexes.end())
	{
		throw FileSystemException("Path does not exist in the archive!");
	}

	const FileData& fd = fileData[iter->second];

	if (fd.type != FILE)
	{
		throw FileSystemException("Entry is no file!");
	}

	res = SzArEx_Extract(&db, &lookStream.s, fd.index, &blockIndex, &outBuffer, &outBufferSize, &offset, &outSizeProcessed, &allocImp, &allocTempImp);
	if (res == SZ_OK)
	{
		boost::shared_array<char> dataPtr(new char[outSizeProcessed]);
		arraySize = outSizeProcessed;

		memcpy(dataPtr.get(), (char*)outBuffer + offset, outSizeProcessed);

		return dataPtr;
	}
	else
	{
		throw FileSystemException(GetErrorStr(res));
	}
}
