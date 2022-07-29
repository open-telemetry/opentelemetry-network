// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/fd_reader.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

FDReader::FDReader(int pid)
    : pid_(pid), task_tid_(-1), fd_dir_(NULL), task_dir_(NULL), task_fd_dir_(NULL), fd_dir_ent_(NULL), task_dir_ent_(NULL)
{}

FDReader::~FDReader()
{
  // XXX: may want to add checks for the return value of closedir
  if (task_dir_)
    closedir(task_dir_);
  if (fd_dir_)
    closedir(fd_dir_);
}

int FDReader::get_inode()
{
  // check if this entry is a fd
  int fd;
  if (sscanf(fd_dir_ent_->d_name, "%d", &fd) != 1)
    return -1; // exit if the current fd_dir_ent_ is either "." or ".."

  // create the link to this fd
  char link[64];
  if (snprintf(link, sizeof(link), "/proc/%d/fd/%d", pid_, fd) < 0)
    return -1;

  // read the link
  char link_content[20];
  int info_len = readlink(link, link_content, sizeof(link_content) - 1);
  if (info_len == -1)
    return -1; // exit if there was an error reading the file
  link_content[info_len] = '\0';

  // check whether this was a socket
  if (strncmp(link_content, "socket:[", strlen("socket:[")))
    return -1; // strncmp(a,b) == 0 when a == b

  // get the inode number
  int ino;
  if (sscanf(link_content, "socket:[%u]", &ino) != 1)
    return -1; // sscanf returns the number of items it matched
  return ino;
}

int FDReader::next_task()
{
  int tid = -1;
  while (tid == -1) {
    task_dir_ent_ = readdir(task_dir_);
    if (task_dir_ent_ == 0)
      return -1; // there are no more entries left
    // check that this dir was not "." or ".."
    int temp; // XXX: not sure that sscanf won't overwrite a value
    if (sscanf(task_dir_ent_->d_name, "%d", &temp) != 1) {
      continue;
    }
    tid = temp;
  }
  task_tid_ = tid;
  return 0;
}

int FDReader::next_fd()
{
  int fd = -1;
  while (fd == -1) {
    fd_dir_ent_ = readdir(fd_dir_);
    if (fd_dir_ent_ == 0)
      return -1;
    int temp; // XXX: not sure that sscanf won't overwrite a value
    if (sscanf(fd_dir_ent_->d_name, "%d", &temp) != 1) {
      continue;
    }
    fd = temp;
  }
  return 0;
}

int FDReader::open_fd_dir()
{
  // create the link to the fd_dir_
  char fd_dir_name[64];
  if (snprintf(fd_dir_name, sizeof(fd_dir_name), "/proc/%d/fd", pid_) < 0)
    return -1;

  // open up the task_dir_
  fd_dir_ = opendir(fd_dir_name);
  if (!fd_dir_)
    return -1;

  return 0;
}

int FDReader::open_task_dir()
{
  // create the link to the task_dir_
  char task_dir_name[64];
  if (snprintf(task_dir_name, sizeof(task_dir_name), "/proc/%d/task", pid_) < 0)
    return -1;

  // open up the task_dir_
  task_dir_ = opendir(task_dir_name);
  if (!task_dir_)
    return -1;

  return 0;
}

int FDReader::open_task_comm()
{
  // create the link to the task comm
  char link[64];
  if (snprintf(link, sizeof(link), "/proc/%d/task/%d/comm", pid_, task_tid_) < 0)
    return -1;

  // read the link
  char link_content[16];
  int info_len = readlink(link, link_content, sizeof(link_content) - 1);
  if (info_len == -1)
    return -1; // exit if there was an error reading the file
  link_content[info_len] = '\0';

  return 0;
}
