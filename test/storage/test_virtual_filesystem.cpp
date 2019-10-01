#include <catch/catch.hpp>

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <storage/VirtualFilesystem.h>

using namespace storage;

namespace tests {
    TEST_CASE("Check loading local files", "[storage]") {
        VirtualFilesystem &vfs = getVirtualFilesystem();
        vfs.clear();

        util::SystemConfig &conf = util::getSystemConfig();
        std::string original = conf.fsMode;

        std::string validPath = "include/python3.7m/Python.h";
        std::string maskedValidPath = maskPath(validPath);
        std::string invalidPath = "/foobar/123/blah";

        SECTION("Valid file but FS mode off") {
            conf.fsMode = "off";
            REQUIRE_THROWS(vfs.openFile(validPath, O_RDONLY, 0));
        }
        SECTION("Valid file with FS mode on") {
            conf.fsMode = "on";

            int fd = vfs.openFile(validPath, O_RDONLY, 0);
            REQUIRE(fd > 0);

            // Read in from the returned descriptor
            std::vector<char> actual(100);
            read(fd, actual.data(), 100);

            // Read in from the actual file
            std::vector<char> expected(100);
            FILE *fp = fopen(maskedValidPath.c_str(), "r");
            fread(expected.data(), 100, 1, fp);

            REQUIRE(actual == expected);
        }
        SECTION("Non-existent with FS mode off") {
            conf.fsMode = "off";
            REQUIRE_THROWS(vfs.openFile(invalidPath, O_RDONLY, 0));
        }
        SECTION("Non-existent with FS mode on") {
            std::string path = "/foobar/123/blah";
            conf.fsMode = "on";
            int fd = vfs.openFile(invalidPath, O_RDONLY, 0);
            REQUIRE(fd == -ENOENT);
        }

        conf.fsMode = original;
    }
}
