#pragma once

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include "VFSPP/defines.hpp"
#include "vfspp_export.h"

namespace vfspp
{
	class IFileSystemEntry;

	typedef std::string string_type;

	typedef boost::shared_ptr<IFileSystemEntry> FileEntryPointer;

	const char DirectorySeparatorChar = '/';

	const char * const DirectorySeparatorStr = "/";

	enum EntryType
	{
		UNKNOWN,
		DIRECTORY,
		FILE
	};

	enum Operations
	{
		OP_READ = 1 << 0,
		OP_WRITE = 1 << 1,
		OP_DELETE = 1 << 2,
		OP_CREATE = 1 << 3
	};

	class VFSPP_EXPORT InvalidOperationException : public std::exception
	{
	public:
		InvalidOperationException(const string_type& message = "Invalid operation!") throw() : msg(message)
		{}

		virtual ~InvalidOperationException() throw() {}

		const char* what() const throw() VFSPP_OVERRIDE { return msg.c_str(); }

	private:
		string_type msg;
	};

	class VFSPP_EXPORT FileSystemException : public std::exception
	{
	public:
		FileSystemException(const string_type& message = "Invalid operation!") throw() : msg(message)
		{}

		virtual ~FileSystemException() throw() {}

		const char* what() const throw() VFSPP_OVERRIDE { return msg.c_str(); }

	private:
		string_type msg;
	};

	class VFSPP_EXPORT IFileSystemEntry
	{
	public:
		enum OpenMode
		{
			MODE_READ = 1 << 0,
			MODE_WRITE = 1 << 1,
			MODE_MEMORY_MAPPED = 1 << 2,
		};

	protected:
		string_type path;

	public:
		IFileSystemEntry(const string_type& pathIn);

		virtual ~IFileSystemEntry() {}

		bool isRoot() const { return path.size() == 0; }

		const string_type& getPath() const { return path; }

		virtual FileEntryPointer getChild(const string_type& path) = 0;

		virtual size_t numChildren() = 0;

		virtual void listChildren(std::vector<FileEntryPointer>& outVector) = 0;

		virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) = 0;

		virtual EntryType getType() const = 0;

		virtual bool deleteChild(const string_type& name) = 0;

		virtual FileEntryPointer createEntry(EntryType type, const string_type& name) = 0;

		virtual void rename(const string_type& newPath) = 0;

		virtual time_t lastWriteTime() = 0;
	};

	class VFSPP_EXPORT IFileSystem
	{
	public:
		virtual ~IFileSystem() {}

		virtual IFileSystemEntry* getRootEntry() = 0;

		virtual int supportedOperations() const = 0;

		virtual string_type getName() const = 0;
	};
}
