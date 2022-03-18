#include "SharedFiles.h"

#include <boost/filesystem.hpp>

#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <faabric/util/string_tools.h>

#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>

namespace storage {
enum FileState
{
    NOT_CHECKED,
    NOT_EXISTS,
    EXISTS_DIR,
    EXISTS
};

static std::shared_mutex sharedFileMapMutex;
static std::unordered_map<std::string, FileState> sharedFileMap;

std::string SharedFiles::prependSharedRoot(const std::string& originalPath)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    boost::filesystem::path p(conf.sharedFilesDir);
    p.append(originalPath);
    return p.string();
}

std::string SharedFiles::realPathForSharedFile(const std::string& sharedPath)
{
    std::string realPath = prependSharedRoot(stripSharedPrefix(sharedPath));
    return realPath;
}

std::string SharedFiles::stripSharedPrefix(const std::string& sharedPath)
{
    std::string strippedPath =
      faabric::util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);
    return strippedPath;
}

bool SharedFiles::isPathShared(const std::string& p)
{
    return faabric::util::startsWith(p, SHARED_FILE_PREFIX);
}

void SharedFiles::deleteSharedFile(const std::string& p)
{
    FileLoader& loader = getFileLoader();
    std::string relativePath = stripSharedPrefix(p);
    loader.deleteSharedFile(relativePath);

    clearCacheForSharedFile(relativePath);
}

void SharedFiles::updateSharedFile(const std::string& p)
{
    FileLoader& loader = getFileLoader();
    std::string relativePath = stripSharedPrefix(p);

    std::vector<uint8_t> bytes = loader.loadSharedFile(relativePath);
    loader.uploadSharedFile(relativePath, bytes);
}

int getReturnValueForSharedFileState(const std::string& sharedPath)
{
    FileState& state = sharedFileMap[sharedPath];
    switch (state) {
        case (NOT_EXISTS): {
            return ENOENT;
        }
        case (EXISTS_DIR):
        case (EXISTS): {
            return 0;
        }
        default: {
            return ENOENT;
        }
    }
}

void SharedFiles::clearCacheForSharedFile(const std::string& sharedPath)
{
    SPDLOG_TRACE("Clearing shared file cache for {}", sharedPath);
    faabric::util::FullLock lock(sharedFileMapMutex);

    sharedFileMap.erase(sharedPath);
}

int SharedFiles::syncSharedFile(const std::string& sharedPath,
                                const std::string& localPath)
{
    // See if file already synced
    {
        faabric::util::SharedLock lock(sharedFileMapMutex);
        if (sharedFileMap.find(sharedPath) != sharedFileMap.end()) {
            if (localPath.empty()) {
                SPDLOG_TRACE("Not syncing shared file {}, already checked",
                             sharedPath);
            } else {
                SPDLOG_TRACE("Not syncing shared file {}, cached at {}",
                             sharedPath,
                             localPath);
            }

            return getReturnValueForSharedFileState(sharedPath);
        }
    }

    // At this point, file has not been synced, therefore need a lock
    faabric::util::FullLock fullLock(sharedFileMapMutex);

    // Check again
    if (sharedFileMap.count(sharedPath) > 0) {
        SPDLOG_TRACE("Not syncing {}, cached at {}", sharedPath, localPath);
        return getReturnValueForSharedFileState(sharedPath);
    }

    if (localPath.empty()) {
        SPDLOG_TRACE("Syncing shared file {}", sharedPath);
    } else {
        SPDLOG_TRACE("Syncing shared file {} to {}", sharedPath, localPath);
    }

    // Work out the real path
    std::string strippedPath =
      faabric::util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);
    std::string realPath;
    if (localPath.empty()) {
        realPath = prependSharedRoot(strippedPath);
    } else {
        realPath = localPath;
    }

    // Check the filesystem
    sharedFileMap[sharedPath] = NOT_CHECKED;
    if (boost::filesystem::exists(realPath)) {
        // If already exists on filesystem, just mark it as such
        if (boost::filesystem::is_directory(realPath)) {
            sharedFileMap[sharedPath] = EXISTS_DIR;
        } else {
            sharedFileMap[sharedPath] = EXISTS;
        }
    } else {
        boost::filesystem::path p(realPath);

        FileLoader& loader = getFileLoader();
        std::vector<uint8_t> bytes;
        bool isDir = false;

        try {
            bytes = loader.loadSharedFile(strippedPath);
        } catch (storage::SharedFileIsDirectoryException& e) {
            isDir = true;
        } catch (storage::SharedFileNotExistsException& e) {
            // Tolerate not existing
        }

        // Handle directories and files accordingly
        if (isDir) {
            // Create directory if path is a directory
            boost::filesystem::create_directories(p);
            sharedFileMap[sharedPath] = EXISTS_DIR;
        } else if (bytes.empty()) {
            sharedFileMap[sharedPath] = NOT_EXISTS;
        } else {
            // Create parent directory
            if (p.has_parent_path()) {
                boost::filesystem::create_directories(p.parent_path());
            }

            // Write to file
            faabric::util::writeBytesToFile(realPath, bytes);
            sharedFileMap[sharedPath] = EXISTS;
        }
    }

    return getReturnValueForSharedFileState(sharedPath);
}

void SharedFiles::syncPythonFunctionFile(const faabric::Message& msg)
{
    if (!msg.ispython()) {
        return;
    }

    SPDLOG_TRACE("Syncing file for Python function {}/{}",
                 msg.pythonuser(),
                 msg.pythonfunction());

    conf::FaasmConfig& conf = conf::getFaasmConfig();
    FileLoader& loader = getFileLoader();
    std::string relativePath = loader.getPythonFunctionRelativePath(msg);

    // This is the shared path of the form faasm:// used to access the Python
    // file as a shared file
    boost::filesystem::path sharedUrl(SHARED_FILE_PREFIX);
    sharedUrl.append(relativePath);

    // This is where we want to write the file to make it _directly_ accessible
    // at runtime.  Python function must be able to access the whole directory,
    // so shared files are insufficient.
    boost::filesystem::path runtimePath(conf.runtimeFilesDir);
    runtimePath.append(relativePath);

    syncSharedFile(sharedUrl.string(), runtimePath.string());
}

void SharedFiles::clear()
{
    sharedFileMap.clear();
}
}
