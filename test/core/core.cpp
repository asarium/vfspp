
#include <VFSPP/util.hpp>

#include <gtest/gtest.h>

using namespace vfspp::util;

TEST(UtilityTest, NormalizePath)
{
	ASSERT_STREQ("", normalizePath("").c_str());

	ASSERT_STREQ("test", normalizePath("test").c_str());
	ASSERT_STREQ("test", normalizePath(  "test").c_str());
	ASSERT_STREQ("test", normalizePath("test  ").c_str());
	ASSERT_STREQ("test", normalizePath("  test  ").c_str());

	ASSERT_STREQ("test", normalizePath("test///").c_str());
	ASSERT_STREQ("test", normalizePath("///test").c_str());
	ASSERT_STREQ("test", normalizePath("///test///").c_str());

	ASSERT_STREQ("test/test", normalizePath("test/test///").c_str());
	ASSERT_STREQ("test/test", normalizePath("///test/test").c_str());
	ASSERT_STREQ("test/test", normalizePath("///test/test///").c_str());
}
