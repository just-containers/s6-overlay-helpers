/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include <skalibs/stat.h>
#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>

#define USAGE "s6-overlay-stat path"

static void print_perms (buffer *b, mode_t mode)
{
  static char const *perms = "oxoworgxgwgruxuwurosgsus" ;
  for (unsigned int i = 0 ; i < 12 ; i++)
    if (mode & (1 << i)) buffer_put(b, perms + (i << 1), 2) ;
}

static inline void dostat (char const *path)
{
  struct passwd *pw ;
  struct group *gr ;
  struct stat st ;
  char fmtu[UID_FMT] ;
  char fmtg[GID_FMT] ;
  if (stat(path, &st) == -1)
    strerr_diefu2sys(111, "stat ", path) ;
  pw = getpwuid(st.st_uid) ;
  gr = getgrgid(st.st_gid) ;
  buffer_put(buffer_1, "uid=", 4) ;
  buffer_put(buffer_1, fmtu, uid_fmt(fmtu, st.st_uid)) ;
  buffer_put(buffer_1, "\nuser=", 6) ;
  if (pw) buffer_puts(buffer_1, pw->pw_name) ;
  buffer_put(buffer_1, "\ngid=", 5) ;
  buffer_put(buffer_1, fmtg, gid_fmt(fmtg, st.st_gid)) ;
  buffer_put(buffer_1, "\ngroup=", 7) ;
  if (gr) buffer_puts(buffer_1, gr->gr_name) ;
  buffer_put(buffer_1, "\nperms=", 7) ;
  print_perms(buffer_1, st.st_mode) ;
  if (!buffer_putflush(buffer_1, "\n", 1))
    strerr_diefu1sys(111, "write to stdout") ;
}

int main (int argc, char const *const *argv)
{
  PROG = "s6-overlay-stat" ;
  if (argc < 2) strerr_dieusage(100, USAGE) ;
  dostat(argv[1]) ;
  return 0 ;
}
