
#include <VFSPP/util.hpp>
#include <VFSPP/memory.hpp>

#include <boost/iostreams/stream_buffer.hpp>

namespace vfspp
{
	namespace memory
	{
		using namespace boost;

		namespace
		{
			template<typename Ch>
			class MemoryBuffer : public boost::iostreams::basic_array<Ch>
			{
			private:
				// We keep this here so the data is deallocated when this object is deleted
				boost::shared_array<Ch> dataPtr;

			public:
				MemoryBuffer(shared_array<Ch> data, size_t n) : dataPtr(data), boost::iostreams::basic_array<Ch>(data.get(), n)
				{
				}
			};
		}

		typedef unordered_map<string_type, size_t>::iterator ChildMapping;

		FileEntryPointer MemoryFileEntry::getChildInternal(const string_type& path)
		{
			size_t separator = path.find_first_of(DirectorySeparatorChar);

			if (separator == string_type::npos)
			{
				ChildMapping found = indexMapping.find(path);

				if (found != indexMapping.end())
				{
					return fileEntries[found->second];
				}
			}
			else
			{
				string_type thisLevel = path.substr(0, separator);

				ChildMapping found = indexMapping.find(thisLevel);

				if (found != indexMapping.end())
				{
					return fileEntries[found->second]->getChildInternal(path.substr(separator + 1));
				}
			}

			return shared_ptr<MemoryFileEntry>();
		}

		FileEntryPointer MemoryFileEntry::getChild(const string_type& path)
		{
			if (type != DIRECTORY)
			{
				throw InvalidOperationException("Entry is no directory!");
			}

			return getChildInternal(util::normalizePath(path));
		}

		size_t MemoryFileEntry::numChildren()
		{
			if (type != DIRECTORY)
			{
				throw InvalidOperationException("Entry is no directory!");
			}

			return fileEntries.size();
		}

		void MemoryFileEntry::listChildren(std::vector<FileEntryPointer>& outVector)
		{
			if (type != DIRECTORY)
			{
				throw InvalidOperationException("Entry is no directory!");
			}

			outVector.clear();

			std::copy(fileEntries.begin(), fileEntries.end(), std::back_inserter(outVector));
		}

		boost::shared_ptr<std::streambuf> MemoryFileEntry::open(int mode)
		{
			if (type != FILE)
			{
				throw InvalidOperationException("Entry is no file!");
			}

			if (mode & MODE_WRITE)
			{
				throw InvalidOperationException("Memory file is read only!");
			}

			if (mode & MODE_MEMORY_MAPPED)
			{
				throw InvalidOperationException("Cannot open in memory mapped mode!");
			}

			return shared_ptr<std::streambuf>(new iostreams::stream_buffer<MemoryBuffer<char>>(MemoryBuffer<char>(data, dataSize)));
		}

		EntryType MemoryFileEntry::getType() const
		{
			return type;
		}

		bool MemoryFileEntry::deleteChild(const string_type& name)
		{
			throw InvalidOperationException("Deleting not supported!");
		}

		FileEntryPointer MemoryFileEntry::createEntry(EntryType type, const string_type& name)
		{
			throw InvalidOperationException("Creating not supported!");
		}

		void MemoryFileEntry::rename(const string_type& newPath)
		{
			throw InvalidOperationException("Renaming not supported!");
		}

		boost::shared_ptr<MemoryFileEntry> MemoryFileEntry::addChild(const string_type& name, EntryType type,
			time_t write_time, void* data, size_t dataSize)
		{
			if (this->type != DIRECTORY)
			{
				throw InvalidOperationException("Can only add children to directory!");
			}

			if (name.find(DirectorySeparatorChar) != string_type::npos)
			{
				throw InvalidOperationException("No child path may be specified!");
			}

			string_type newPath(path);
			newPath.append(DirectorySeparatorStr).append(util::normalizePath(name));

			shared_ptr<MemoryFileEntry> entry = shared_ptr<MemoryFileEntry>(new MemoryFileEntry(name));

			entry->type = type;
			entry->writeTime = write_time;

			if (type == FILE)
			{
				entry->data = shared_array<char>(new char[dataSize]);
				memcpy(entry->data.get(), data, dataSize);
				entry->dataSize = dataSize;
			}

			fileEntries.push_back(entry);
			indexMapping.insert(std::make_pair(util::normalizePath(name), fileEntries.size() - 1));

			return entry;
		}
	}
}
