/*
  https://github.com/kddeisz/tree

  GnUeFtwPkg - https://github.com/cecekpawon/GnUeFtwPkg
  cecekpawon - Wed Aug 14 15:57:45 2019
*/

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <stdio.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct {
  int   dir_count;
  int   file_count;
} counter_t;


counter_t *
counter_build (
  void
  )
{
  counter_t   *counter;

  counter = (counter_t *) malloc (sizeof (counter_t));

  if (counter != NULL) {
    counter->dir_count  = 0;
    counter->file_count = 0;
  }

  return counter;
}

void
counter_incr_dirs (
  counter_t   *counter
  )
{
  if (counter != NULL) {
    counter->dir_count += 1;
  }
}

void
counter_incr_files (
  counter_t   *counter
  )
{
  if (counter != NULL) {
    counter->file_count += 1;
  }
}

void
counter_free (
  counter_t   *counter
  )
{
  if (counter != NULL) {
    free (counter);
  }
}

void
bubble_sort (
  int   size,
  char  **entries
  )
{
  int   x_idx;
  int   size2;

  size2 = size - 1;

  for (x_idx = 0; x_idx < size; x_idx++) {
    int   y_idx;

    for (y_idx = 0; y_idx < size2; y_idx++) {
      if (strcmp (entries[y_idx], entries[y_idx + 1]) > 0) {
        char  *swap;

        swap = entries[y_idx];
        entries[y_idx] = entries[y_idx + 1];
        entries[y_idx + 1] = swap;
      }
    }
  }
}

int
dir_entry_count (
  const char  *directory
  )
{
         DIR    *dir_handle;
         int    count;
  struct dirent *file_dirent;

  dir_handle = opendir (directory);

  if (dir_handle == NULL) {
    printf ("Cannot open directory \"%s\"\n", directory);
    return -1;
  }

  count = 0;

  while ((file_dirent = readdir (dir_handle)) != NULL) {
    wchar_t   *uname;
    size_t    len;

    uname = file_dirent->d_name;
    len   = wcslen (uname);

    if ((len == 0) || ((uname[0] == L'.') && ((len == 1) || ((len == 2) && (uname[1] == L'.'))))) {
      continue;
    }

    count++;
  }

  closedir (dir_handle);

  return count;
}

char *
join (
  const char  *left,
  const char  *right
  )
{
  char  *result;

  result = malloc (((strlen (left) + strlen (right)) + 1) * 2 /*safety uni sym*/);

  if (result != NULL) {
    strcpy (result, left);
    strcat (result, right);
  }

  return result;
}

char *
path_join (
  const char  *directory,
  const char  *entry
  )
{
  char  *result;

  result = malloc (strlen (directory) + strlen (entry) + 2);

  if (result != NULL) {
    strcpy (result, directory);
    strcat (result, "\\");
    strcat (result, entry);
  }

  return result;
}

int
is_dir (
  const char  *entry
  )
{
  struct stat  entry_stat;

  if ((stat (entry, &entry_stat) != 0) || !S_ISDIR (entry_stat.st_mode)) {
    return 0;
  }

  return 1;
}

int
walk (
  const char        *directory,
  const char        *prefix,
        counter_t   *counter
  )
{
          int     entry_count;
          DIR     *dir_handle;
          char    **entries;
          int     entry_idx;
  struct  dirent  *file_dirent;

  entry_count = dir_entry_count (directory);

  if (entry_count == -1) {
    //print ("\n");
    return -1;
  }

  dir_handle = opendir (directory);

  if (dir_handle == NULL) {
    //print ("\n");
    return -1;
  }

  entries = malloc (sizeof (char *) * entry_count);

  if (entries == NULL) {
    //print ("\n");
    closedir (dir_handle);
    return -1;
  }

  entry_idx = 0;

  counter_incr_dirs (counter);

  while ((file_dirent = readdir (dir_handle)) != NULL) {
    wchar_t   *uname;
    size_t    len;
    char      *aname;

    uname = file_dirent->d_name;
    len   = wcslen (uname);

    if ((len == 0) || ((uname[0] == L'.') && ((len == 1) || ((len == 2) && (uname[1] == L'.'))))) {
      continue;
    }

    if ((aname = malloc (NAME_MAX)) != NULL) {
      if (wcstombs (aname, uname, NAME_MAX) == -1) {
        free (aname);
        continue;
      }

      entries[entry_idx++] = aname;
    }
  }

  closedir (dir_handle);

  if (entry_idx == 0) {
    //print ("\n");
    free (entries);
    return -1;
  }

  bubble_sort (entry_count, entries);

  for (entry_idx = 0; entry_idx < entry_count; entry_idx++) {
    char  *full_path;
    char  *pointer;
    char  *prefix_ext;

    if (entry_idx == entry_count - 1) {
      pointer     = "└── ";
      prefix_ext  = "    ";
    }
    else {
      pointer     = "├── ";
      prefix_ext  = "│   ";
    }

    printf ("%s%s%s\n", prefix, &pointer[0], entries[entry_idx]);

    full_path = path_join (directory, entries[entry_idx]);

    if (full_path != NULL) {
      if (is_dir (full_path)) {
        prefix_ext = join (prefix, prefix_ext);

        if (prefix_ext != NULL) {
          walk (full_path, prefix_ext, counter);
          free (prefix_ext);
        }
      }
      else {
        counter_incr_files (counter);
      }

      free (full_path);
    }

    free (entries[entry_idx]);
  }

  free (entries);

  return 0;
}

int
main (
  int   argc,
  char  *argv[]
  )
{
  char        *directory;
  counter_t   *counter;

  directory = ".";

  //
  // TODO: apply max-depth, file-type
  //

  if (argc > 1) {
    if (strlen (argv[1]) != 0) {
      directory = argv[1];
    }
  }

  printf ("%s\n", directory);

  counter = counter_build ();

  if (counter != NULL) {
    size_t  i;
    size_t  len;

    len = strlen (directory);

    for (i = 0; i < len; i++) {
      if (directory[i] == '/') {
        directory[i] = '\\';
      }
    }

    walk (directory, "", counter);

    printf ("\n%d directories, %d files\n", counter->dir_count - 1, counter->file_count);

    counter_free (counter);
  }

  return 0;
}
