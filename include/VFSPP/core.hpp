#pragma once

#include <string>
#include <vector>

#include <VFSPP/defines.hpp>
#include <vfspp_export.h>

#include <boost/smart_ptr.hpp>

namespace vfspp
{
	typedef std::string string_type;

	const char DirectorySeparatorChar = '/';

	const char * const DirectorySeparatorStr = "/";

	string_type normalizePath(const string_type& inPath);

	int modeToOperation(int mode);

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
		};

	protected:
		string_type path;

	public:
		IFileSystemEntry(const string_type& pathIn);

		virtual ~IFileSystemEntry() {}

		bool isRoot() const { return path.size() == 0; }

		const string_type& getPath() const { return path; }

		virtual boost::shared_ptr<IFileSystemEntry> getChild(const string_type& path) = 0;

		virtual size_t numChildren() = 0;

		virtual void listChildren(std::vector<boost::shared_ptr<IFileSystemEntry> >& outVector) = 0;

		virtual boost::shared_ptr<std::streambuf> open(int mode = MODE_READ) = 0;

		virtual EntryType getType() const = 0;

		virtual bool deleteChild(const string_type& name) = 0;

		virtual boost::shared_ptr<IFileSystemEntry> createEntry(EntryType type, const string_type& name) = 0;
	};

	class VFSPP_EXPORT IFileSystem
	{
	public:
		virtual ~IFileSystem() {}

		virtual IFileSystemEntry* getRootEntry() = 0;

		virtual int supportedOperations() const = 0;
	};
}
