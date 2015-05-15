
#pragma once

#include <VFSPP/core.hpp>

#include <boost/unordered_map.hpp>

#include <boost/filesystem/path.hpp>

namespace vfspp
{
	namespace util
	{
		template<class InType, class OutType = InType>
		OutType normalizePath(const InType& inPath, bool lowerCase = false)
		{
			OutType outPath(inPath);

			boost::trim(outPath);

			boost::trim_if(outPath, boost::is_any_of(DirectorySeparatorStr));

			if (lowerCase)
			{
				boost::to_lower(outPath);
			}

			return outPath;
		}

		inline string_type normalizePath(const char* inPath, bool lowerCase = false)
		{
			return normalizePath<const char*, string_type>(inPath, lowerCase);
		}

		int modeToOperation(int mode);

		template<typename DataType>
		class VFSPP_EXPORT ArchiveFileSystem : public IFileSystem
		{
		public:
			ArchiveFileSystem(const boost::filesystem::path& filePathIn) : filePath(filePathIn) {}

			virtual ~ArchiveFileSystem() {}

			virtual IFileSystemEntry* getRootEntry() = 0;

			virtual int supportedOperations() const = 0;

			virtual string_type getName() const { return filePath.string(); }

		protected:
			typedef DataType data_type;

			boost::filesystem::path filePath;
			
			std::vector<data_type> fileData;
			boost::unordered_map<string_type, size_t> fileIndexes;

			void addFileData(const string_type& path, const data_type& data)
			{
				fileData.push_back(data);
				fileIndexes.insert(std::make_pair(path, fileData.size() - 1));
			}

			data_type getFileData(const string_type& path) const
			{
				if (path.length() == 0)
				{
					// Special case: The root entry is always a directory
					data_type data;
					data.type = DIRECTORY;

					return data;
				}

				boost::unordered_map<string_type, size_t>::const_iterator iter = fileIndexes.find(path);

				if (iter == fileIndexes.end())
				{
					return data_type();
				}
				else
				{
					return fileData[iter->second];
				}
			}
		};
	}
}
