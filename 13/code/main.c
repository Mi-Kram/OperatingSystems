#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BASE_DIR "/tmp/kramarenko-dz13"
#define BASE_FILE "a"

void dispose();

int main() {
  // create temp directory.
  if (mkdir(BASE_DIR, 0666) < 0 && errno != EEXIST) {
    perror(BASE_DIR);
    return 0;
  }

  char base_path[PATH_MAX];
  snprintf(base_path, sizeof(base_path), BASE_DIR "/" BASE_FILE);

  // create base file to link to.
  int fd = open(base_path, O_CREAT | O_WRONLY, 0666);
  if (fd < 0) {
    perror(base_path);
    dispose();
    return 0;
  }
  close(fd);

  int depth = 0;

  // store previous path to link to.
  char prev_path[PATH_MAX];
  snprintf(prev_path, sizeof(prev_path), "%s", base_path);

  while (1) {
    char link_path[PATH_MAX];
    snprintf(link_path, sizeof(link_path), BASE_DIR "/link_%d", depth);

    // create new link.
    if (symlink(prev_path, link_path) < 0) {
      perror("symlink");
      break;
    }

    // try to open new link.
    int fd = open(link_path, O_RDONLY);
    if (fd < 0) {
      printf("Max depth: %d\n", depth);
      unlink(link_path);
      break;
    }
    close(fd);

    // update previous path and increase depth.
    snprintf(prev_path, sizeof(prev_path), "%s", link_path);
    ++depth;
  }

  dispose();
  return 0;
}

void dispose() {
  // open temp directory.
  DIR *dir = opendir(BASE_DIR);
  if (!dir) return;

  struct dirent *entry;
  char path[PATH_MAX];

  // for each entry in directory.
  while ((entry = readdir(dir)) != NULL) {
    // skip system directories "." and "..".
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

    // remove link.
    snprintf(path, sizeof(path), BASE_DIR "/%s", entry->d_name);
    unlink(path);
  }

  // remove temp directory.
  closedir(dir);
  rmdir(BASE_DIR);
}

