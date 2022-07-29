/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dirent.h>

/**
 * Does two things:
 * 1. Reads over the task directory of a given pid
 *    i.e. /proc/[pid]/task
 * 2. Reads over the entries in the fd directory of a given pid
 *    i.e. /proc/[pid]/fd
 * We report the inode for entries in the fd directory that are sockets
 */
class FDReader {
public:
  /**
   * c'tor
   */
  FDReader(int pid);

  /**
   * d'tor
   */
  ~FDReader();

  /**
   * Returns the inode number if the fd is for a socket
   * Returns -1 if the this fd was not a socket
   */
  int get_inode();

  /**
   * Read task_dir_. Returns 0 if a task was found and -1 otherwise.
   * i.e. skips over non-task entries until we find a task or run out of entries
   * Sets the task_dir_ent_ and task_tid_ if we do find an entry
   *
   */
  int next_task();

  /**
   * f_dir_. Returns 0 if an entry was found and -1 otherwise.
   * i.e. skips over non-task entries until we find a task or run out of entries
   * Sets the fd_dir_ent_ if we do find an entry
   */
  int next_fd();

  /**
   * Opens the fd directory specified by pid_.
   * Returns 0 on success, -1 if failed.
   */
  int open_fd_dir();

  /**
   * Opens the task directory specified by pid_.
   * Returns 0 on success, -1 if failed.
   */
  int open_task_dir();

  /**
   * Opens the comm file specified by pid_ and tid_.
   * Returns 0 on success, -1 if failed.
   */
  int open_task_comm();

private:
  int pid_;
  int task_tid_;
  DIR *fd_dir_;      // /proc/[pid]/fd
  DIR *task_dir_;    // /proc/[pid]/task
  DIR *task_fd_dir_; // /proc/[pid]/task/[tid]/fd
  dirent *fd_dir_ent_;
  dirent *task_dir_ent_;
};
