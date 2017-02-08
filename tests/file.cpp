
/* Copyright (c) 2014-2016, Daniel.Nachbaur@epfl.ch
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <lunchbox/test.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <lunchbox/file.h>

int main(int, char** argv)
{
    const boost::filesystem::path path(argv[0]);
    const std::string argvPath(path.parent_path().generic_string());
    const boost::filesystem::path execPath(lunchbox::getExecutableDir());
    TEST(boost::algorithm::ends_with(execPath.generic_string(), argvPath));

    boost::filesystem::path referenceRootPath(execPath);
    referenceRootPath = referenceRootPath.parent_path();
#ifdef _MSC_VER
    const lunchbox::Strings buildTypes{"debug", "relwithdebinfo", "release",
                                       "minsizerel"};
    std::string buildType(path.stem().string());
    std::transform(buildType.begin(), buildType.end(), buildType.begin(),
                   ::tolower);
    if (std::find(buildTypes.begin(), buildTypes.end(), buildType) !=
        buildTypes.end())
    {
        referenceRootPath = referenceRootPath.parent_path();
    }
#endif
    TEST(lunchbox::getRootDir() == referenceRootPath.string());
    TEST(lunchbox::getExecutableDir() == lunchbox::getWorkDir());

    const std::string filename = path.filename().generic_string();
    TEST(filename == lunchbox::getFilename(argv[0]));

    const lunchbox::Strings files = lunchbox::searchDirectory(argvPath, ".*");
    TEST(files.size() > 1);
    TEST(std::find(files.begin(), files.end(), filename) != files.end());

    return EXIT_SUCCESS;
}
