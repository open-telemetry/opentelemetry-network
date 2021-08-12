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

#pragma once

// This file contains helper functions for file- and filesystem-based
// operations.

#include <util/expected.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <initializer_list>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <cstdint>

constexpr auto MAX_PID_PROC_PATH = "/proc/sys/kernel/pid_max";

enum class FileAccess : int { read = R_OK, write = W_OK, execute = X_OK };

// Tells whether the file specified by `path` exists
bool file_exists(char const *path, std::initializer_list<FileAccess> modes = {});

// Creates the directory `directory` with user read/write/exec permissions.
// Returns `directory` if the directory was created or already existed, or the
// error code if the directory failed to be created.
Expected<char const *, std::error_code> create_directory(char const *directory);

// Struct containing several pieces of file metadata. Feel free to extend this
// with new fields if desired.
struct FileMeta {
  std::string path;
  int64_t size_bytes = 0;
  int64_t modify_nanotimestamp = 0;

  // Constructs a FileMeta object from a stat result.
  static FileMeta from_stat(std::string path, const struct stat &statbuf);
};

// Lists file contents of `dir`, with no guarantee of ordering. Will return an
// empty list upon read failure.
// TODO: Better error handling if it becomes significant.
std::vector<FileMeta> list_directory_contents(char const *directory);

// Cleans up the contents of `dir` such that it contains no more than
// `max_file_count` files and a total size less than `max_total_size_bytes`.
// Removes older files first.
void cleanup_directory(char const *directory, int64_t max_file_count, int64_t max_total_size_bytes);

// reads the contents of given file - no decoding is performed
// returns the file contents on success, or the error information otherwise
Expected<std::vector<std::uint8_t>, std::error_code> read_file(char const *path);

// reads the contents of given file - no decoding is performed
// returns the file contents on success, or the error information otherwise
Expected<std::string, std::error_code> read_file_as_string(char const *path);

// reads the contents of given file into the given buffer
// no decoding is performed, the string's characters represent the file's bytes
// returns the amount of bytes read on success, or the error information otherwise
// note: the buffer won't be cleared beforehand, the file's contents will be appended
Expected<std::size_t, std::error_code> read_file(char const *path, std::vector<std::uint8_t> &buffer);

std::error_code write_file(char const *path, std::string_view data);

class FileDescriptor {
public:
  static constexpr int INVALID_FD = -1;

  FileDescriptor() : fd_(INVALID_FD) {}

  explicit FileDescriptor(int fd) : fd_(fd) {}

  FileDescriptor(FileDescriptor const &) = delete;
  FileDescriptor(FileDescriptor &&other) : fd_(other.fd_) { other.fd_ = INVALID_FD; }

  ~FileDescriptor();

  enum class Access : int { read_only = (O_RDONLY), write_only = (O_WRONLY), read_write = (O_RDWR) };

  enum class Positioning : int { beginning = 0, append = (O_APPEND), truncate = (O_TRUNC) };

  enum class Mode : int { none = 0, close_on_exec = (O_CLOEXEC) };

  enum class Permission : int {
    none = 0,
    read = 1,
    write = 2,
    read_write = (read | write),
    exec = 4,
    read_exec = (read | exec),
    write_exec = (write | exec),
    read_write_exec = (read | write | exec)
  };

  // reads the contents of this file into the given buffer
  // performs as many read operations as possible in order to fill the buffer
  // no decoding is performed, the file's bytes are returned raw
  // returns the amount of bytes read on success, or the error information otherwise
  // if less than `size` bytes were returned then this function has reached EOF
  Expected<std::size_t, std::error_code> read_all(char *buffer, std::size_t size);

  // reads the contents of this file into the given buffer, growing it as needed
  // no decoding is performed, the file's bytes are returned raw
  // returns the amount of bytes read on success, or the error information otherwise
  Expected<std::size_t, std::error_code> read_all(std::vector<std::uint8_t> &out);

  // writes the contents of `buffer` to file `fd`
  // returns 0 on success or an error code on failure (see write(2) for details)
  std::error_code write_all(std::string_view buffer);

  int fd() const { return fd_; }
  bool valid() const { return fd_ != INVALID_FD; }

  // flushes modified data but avoids flushing metadata - fdatasync(2)
  // the return value evaluates to false on success or represents the error otherwise
  std::error_code flush_data();

  // flushes modified data and metadata - fsync(2)
  // the return value evaluates to false on success or represents the error otherwise
  std::error_code flush();

  int operator*() const { return fd(); }
  explicit operator bool() const { return valid(); }
  bool operator!() const { return !valid(); }

  FileDescriptor &operator=(FileDescriptor &&other)
  {
    if (valid()) {
      close();
    }

    std::swap(fd_, other.fd_);
    return *this;
  }

  std::error_code
  open(char const *path, Access access, Positioning position = Positioning::beginning, Mode mode = Mode::close_on_exec);
  std::error_code create(
      char const *path,
      Access access,
      Positioning position = Positioning::truncate,
      Permission user_permission = Permission::read_write,
      Permission group_permission = Permission::read_write,
      Permission others_permission = Permission::none,
      Mode mode = Mode::close_on_exec);

  std::error_code close();

  Expected<std::size_t, std::error_code> read(void *buffer, std::size_t size);

  static FileDescriptor std_in() { return FileDescriptor{0}; }
  static FileDescriptor std_out() { return FileDescriptor{1}; }
  static FileDescriptor std_err() { return FileDescriptor{2}; }

private:
  int fd_;
};
