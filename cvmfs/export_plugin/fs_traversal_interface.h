/**
 * This file is part of the CernVM File System.
 */
#ifndef CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_INTERFACE_H_
#define CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_INTERFACE_H_

#include "hash.h"
#include "pointer.h"
#include "shortstring.h"

enum fs_type {
  FS_CVMFS,
  FS_POSIX,
  FS_SQUASH,
};

struct fs_traversal_context {
  uint64_t version;
  uint64_t size;
  fs_type type;

  char *repo;
  char *data;

  void * ctx;
};

enum fs_open_type {
  fs_open_read = 1,
  fs_open_write = 2,
  fs_open_append = 4
};

struct fs_file {
  uint64_t version;
  uint64_t size;
  void *ctx;

  struct fs_stat *stat_info;

  int (*open)(void *ctx, fs_open_type op_mode);
  int (*close)(void *ctx);
  int (*read)(void *ctx, char *buff, size_t len);
  int (*write)(void *ctx, const char *buff);
};

struct fs_traversal {

  struct fs_traversal_context *ctx;

  // NOTE(steuber): How does this work?
  struct fs_traversal *(*initialize(fs_type type, const char *repo, const char *data));

  void (*finalize)(struct fs_traversal_context *ctx);

  /**
   * Method which returns a list over the given directory
   * 
   * @param[in] dir The directory over which should be iterated
   * @param[out] buf The list of the paths to the elements in the directory
   * @param[in] len @todo(steuber): what?
   */
  void (*list_dir)(struct fs_traversal_context *ctx,
                const char *dir,
                char ***buf,
                size_t len);

  /**
   * Method which returns a stat struct given a file path.
   * If the file doesn't exist, NULL is returned
   * 
   * @param[in] path The path of the object to stat
   */
  struct cvmfs_stat *(get_stat)(struct fs_traversal_context *ctx,
                const char *path);

  /**
   * Method which checks whether the file described by the given content and metadata hash
   * exists in the destination file system
   * 
   * This should always be realised by a file system lookup since the all files
   * should be hardlinked once by a combination of content and metadata hash.
   * 
   * @param[in] content The content hash of the file
   * @param[in] meta The meta hash of the file
   * @returns True if file was found, false if not
   */
  bool (*has_hash)(struct fs_traversal_context *ctx,
                const void *content,
                const void *meta);

  /**
   * Method which creates a hardlink from the given path to the file identified
   * by its content hash.
   * 
   * For this call to succeed the file addressed by the content hash already
   * needs to exist in the destination file system
   * 
   * Error if:
   * - Hash file does not exist
   * 
   * @param[in] content The content hash of the file
   * @param[in] meta The meta hash of the file
   * @param[in] meta The meta hash of the file to hardlink
   */
  int (*link)(struct fs_traversal_context *ctx,
                const char *path,
                void *content,
                void *meta);

  /**
   * Method removes the hardlink at the given path
   * 
   * Error if:
   * - unlink not successful
   * 
   * @param[in] path The path which should be removed
   */
  int (*unlink)(struct fs_traversal_context *ctx,
                const char *path);

  /**
   * Method which will create the given directory
   * 
   * @todo(steuber): Fail if "deep" mkdir?
   * 
   * @param[in] path The path to the directory that should be created
   * @param[in] stat The stat containing the meta data for directory creation
   */
  int (*mkdir)(struct fs_traversal_context *ctx,
                const char *path,
                const struct cvmfs_stat *stat);

  /**
   * Method which removes the given directory
   * 
   * @param[in] path The path to the directory that should be removed
   */
  int (*rmdir)(struct fs_traversal_context *ctx,
                const char *path);

  /**
   * Atomically creates the file representing
   * the given content and meta data hash
   * 
   * @param[in] content The content hash of the file
   * @param[in] meta The meta hash of the file
   * @param[in] stat The stat containing the meta data for directory creation
   */
  int (*touch)(struct fs_traversal_context *ctx,
                void *content,
                void *meta,
                const struct cvmfs_stat *stat);

  /**
   * Retrieves a method struct which allows the manipulation of the file
   * defined by the given content and meta data hash
   * 
   * @param[in] content The content hash of the file
   * @param[in] meta The meta hash of the file
   */
  struct fs_file *(*get_handle)(struct fs_traversal_context *ctx,
                void *content,
                void *meta);


  /**
   * Method which creates a symlink at src which points to dest
   * 
   * @param[in] The position at which the symlink should be saved
   * (parent directory must exist)
   * @param[in] The position the symlink should point to
   */
  int (*symlink)(struct fs_traversal_context *ctx,
                const char *src,
                const char *dest);

  /**
   * Method which executes a garbage collection on the destination file system.
   * This will remove all no longer linked content adressed files
   */
  // NOTE(steuber): Shouldn't this maybe just be part of the finalize step?
  int (*garbage_collection)(struct fs_traversal_context *ctx);
};

#endif  // CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_INTERFACE_H_