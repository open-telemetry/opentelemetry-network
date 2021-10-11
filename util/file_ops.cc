//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <util/file_ops.h>

#include <util/defer.h>
#include <util/log.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <string>
#include <vector>

#include <cassert>
#include <cstdint>

namespace {

// RAII-based directory closer.
struct DirectoryCloser {
  DIR *const dp;
  ~DirectoryCloser()
  {
    if (dp)
      closedir(dp);
  }
};

} // namespace

bool file_exists(char const *path, std::initializer_list<FileAccess> modes)
{
  int mode = F_OK;
  for (auto i : modes) {
    mode |= static_cast<int>(i);
  }
  return access(path, mode) == 0;
}

Expected<char const *, std::error_code> create_directory(char const *directory)
{
  if (auto const error = ::mkdir(directory, S_IRWXU); !error || errno == EEXIST) {
    return directory;
  } else {
    return {unexpected, std::error_code{errno, std::generic_category()}};
  }
}

FileMeta FileMeta::from_stat(std::string path, const struct stat &statbuf)
{
  FileMeta info;
  info.path = std::move(path);
  info.size_bytes = statbuf.st_size;
  info.modify_nanotimestamp = statbuf.st_mtim.tv_sec * 1000000000LL + statbuf.st_mtim.tv_nsec;
  return info;
}

std::vector<FileMeta> list_directory_contents(char const *directory)
{
  std::vector<FileMeta> result;

  DIR *const dp = opendir(directory);
  DirectoryCloser closer = {dp};

  if (dp) {
    // Get the list of file names in the directory.
    std::vector<std::string> file_names;
    struct dirent *entry;
    while ((entry = readdir(dp)) != nullptr) {
      file_names.emplace_back(entry->d_name);
    }

    // Stat each file to get metadata.
    for (const std::string &file_name : file_names) {
      auto full_path = fmt::format("{}/{}", directory, file_name);
      struct stat statbuf;

      const int status = stat(full_path.c_str(), &statbuf);
      if (status < 0) {
        LOG::warn("Failed to stat file {} : {}", full_path, strerror(errno));
        continue;
      }

      // Only include regular files.
      if ((statbuf.st_mode & S_IFREG) == 0)
        continue;

      result.push_back(FileMeta::from_stat(std::move(full_path), statbuf));
    }
  }

  return result;
}

void cleanup_directory(char const *directory, const int64_t max_file_count, const int64_t max_total_size_bytes)
{
  // Get the contents of dir in reverse chronological order.
  std::vector<FileMeta> files = list_directory_contents(directory);
  std::sort(files.begin(), files.end(), [](const FileMeta &lhs, const FileMeta &rhs) {
    return lhs.modify_nanotimestamp > rhs.modify_nanotimestamp;
  });

  // Find the list of files to delete.
  std::vector<const FileMeta *> files_to_delete;
  int64_t file_count = 0;
  int64_t total_size_bytes = 0;
  for (const FileMeta &file : files) {
    file_count += 1;
    total_size_bytes += file.size_bytes;

    if (file_count > max_file_count || total_size_bytes > max_total_size_bytes) {
      files_to_delete.push_back(&file);
    }
  }

  // Delete the files.
  for (const FileMeta *file : files_to_delete) {
    const int status = remove(file->path.c_str());
    if (status == 0 /* success */) {
      LOG::info("Deleted file {}", file->path);
    } else {
      LOG::warn("Failed to delete file {} : {}", file->path, strerror(errno));
    }
  }
}

Expected<std::vector<std::uint8_t>, std::error_code> read_file(char const *path)
{
  std::vector<std::uint8_t> buffer;

  if (auto result = read_file(path, buffer); !result) {
    return {unexpected, std::move(result.error())};
  }

  // legit move - NRVO wouldn't apply because the return type differs
  return std::move(buffer);
}

Expected<std::string, std::error_code> read_file_as_string(char const *path)
{
  auto result = read_file(path);
  if (!result) {
    return {unexpected, result.error()};
  }
  return std::string(reinterpret_cast<char const *>(result->data()), result->size());
}

Expected<std::size_t, std::error_code> read_file(char const *path, std::vector<std::uint8_t> &buffer)
{
  FileDescriptor fd;

  if (auto error = fd.open(path, FileDescriptor::Access::read_only, FileDescriptor::Positioning::beginning)) {
    return {unexpected, std::move(error)};
  }

  if (auto const result = fd.read_all(buffer)) {
    return *result;
  } else {
    return {unexpected, result.error()};
  }
}

std::error_code write_file(char const *path, std::string_view data)
{
  FileDescriptor fd;

  if (auto error = fd.create(path, FileDescriptor::Access::write_only)) {
    return error;
  }

  return fd.write_all(data);
}

FileDescriptor::~FileDescriptor()
{
  if (valid()) {
    close();
  }
}

Expected<std::size_t, std::error_code> FileDescriptor::read(void *buffer, std::size_t size)
{
  ssize_t const result = ::read(fd_, buffer, size);

  if (result < 0) {
    return {unexpected, std::error_code(errno, std::generic_category())};
  }

  return result;
}

Expected<std::size_t, std::error_code> FileDescriptor::read_all(char *buffer, std::size_t size)
{
  auto const begin = buffer;

  for (auto const end = buffer + size;;) {
    assert(buffer <= end);
    if (auto const partial = ::read(fd_, buffer, end - buffer); partial < 0) {
      return {unexpected, std::error_code(errno, std::generic_category())};
    } else if (partial) {
      buffer += partial;
    } else {
      // EOF reached
      break;
    }
  }

  return buffer - begin;
}

Expected<std::size_t, std::error_code> FileDescriptor::read_all(std::vector<std::uint8_t> &out)
{
  std::size_t const offset = out.size();

  // failure to stat doesn't prevent attempting to read the file
  // it simply prevents pre-allocating the whole buffer at once
  if (struct stat info; !::fstat(fd_, &info)) {
    out.resize(offset + info.st_size);
  }

  constexpr std::size_t const chunk_size = 1024;
  std::size_t total = offset;

  for (;;) {
    // should only happen if stat fails or if the file grows after that
    if (out.size() == total) {
      out.resize(total + chunk_size);
    }

    assert(total < out.size());
    if (auto const partial = ::read(fd_, out.data() + total, out.size() - total); partial < 0) {
      return {unexpected, std::error_code(errno, std::generic_category())};
    } else if (partial) {
      total += partial;
    } else {
      // EOF reached
      break;
    }
  }

  // truncate excess buffer
  out.resize(total);

  return total - offset;
}

std::error_code FileDescriptor::write_all(std::string_view buffer)
{
  auto begin = buffer.data();
  auto const end = begin + buffer.size();

  while (begin < end) {
    auto const size = end - begin;
    auto const written = ::write(fd_, begin, size);

    if (written < 0) {
      return std::error_code(errno, std::generic_category());
    }

    assert(written <= size);
    begin += written;
  }

  return {};
}

std::error_code FileDescriptor::flush_data()
{
  return std::error_code{::fdatasync(fd_) ? errno : 0, std::generic_category()};
}

std::error_code FileDescriptor::flush()
{
  return std::error_code{::fsync(fd_) ? errno : 0, std::generic_category()};
}

std::error_code FileDescriptor::close()
{
  auto const error = ::close(fd_) ? errno : 0;
  fd_ = INVALID_FD;
  return {error, std::generic_category()};
}

namespace {

static constexpr int USER_PERMISSIONS_SELECTOR[] = {
    0, (S_IRUSR), (S_IWUSR), (S_IRUSR | S_IWUSR), (S_IXUSR), (S_IRUSR | S_IXUSR), (S_IWUSR | S_IXUSR), (S_IRWXU)};

static constexpr int GROUP_PERMISSIONS_SELECTOR[] = {
    0, (S_IRGRP), (S_IWGRP), (S_IRGRP | S_IWGRP), (S_IXGRP), (S_IRGRP | S_IXGRP), (S_IWGRP | S_IXGRP), (S_IRWXG)};

static constexpr int OTHERS_PERMISSIONS_SELECTOR[] = {
    0, (S_IROTH), (S_IWOTH), (S_IROTH | S_IWOTH), (S_IXOTH), (S_IROTH | S_IXOTH), (S_IWOTH | S_IXOTH), (S_IRWXO)};

} // namespace

std::error_code FileDescriptor::open(char const *path, Access access, Positioning position, Mode mode)
{
  assert(!valid());

  int const flags = static_cast<int>(access) | static_cast<int>(position) | static_cast<int>(mode);

  fd_ = ::open(path, flags);

  return std::error_code{fd_ == INVALID_FD ? errno : 0, std::generic_category()};
}

std::error_code FileDescriptor::create(
    char const *path,
    Access access,
    Positioning position,
    Permission user_permission,
    Permission group_permission,
    Permission others_permission,
    Mode mode)
{
  assert(!valid());

  int const flags = static_cast<int>(access) | static_cast<int>(position) | static_cast<int>(mode) | (O_CREAT);

  int const permissions = USER_PERMISSIONS_SELECTOR[static_cast<int>(user_permission)] |
                          GROUP_PERMISSIONS_SELECTOR[static_cast<int>(group_permission)] |
                          OTHERS_PERMISSIONS_SELECTOR[static_cast<int>(others_permission)];

  fd_ = ::open(path, flags, permissions);

  return std::error_code{fd_ == INVALID_FD ? errno : 0, std::generic_category()};
}
