/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/bytestr.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>
#include <skalibs/direntry.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

#define USAGE "justc-envdir [ -I | -i ] dir prog..."

static int justc_envdir (char const *path, stralloc *modifs)
{
  stralloc contents = STRALLOC_ZERO ;
  unsigned int n = 0 ;
  size_t pathlen = strlen(path) ;
  size_t modifbase = modifs->len ;
  int wasnull = !modifs->s ;
  DIR *dir ;
  dir = opendir(path) ;
  if (!dir) return -1 ;
  for (;;)
  {
    direntry *d ;
    size_t len ;
    contents.len = 0 ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.') continue ;
    len = strlen(d->d_name) ;
    if (str_chr(d->d_name, '=') < len) continue ;
    {
      char tmp[pathlen + len + 2] ;
      memcpy(tmp, path, pathlen) ;
      tmp[pathlen] = '/' ;
      memcpy(tmp + pathlen + 1, d->d_name, len + 1) ;
      if (!openslurpclose(&contents, tmp))
      {
        if (errno == ENOENT) errno = EIDRM ;
        goto err ;
      }
    }
    if (!stralloc_0(&contents)) goto err ;
    if (!env_addmodif(modifs, d->d_name, contents.s)) goto err ;
    n++ ;
  }
  if (errno) goto err ;
  dir_close(dir) ;
  stralloc_free(&contents);
  return n ;

 err:
  {
    int e = errno ;
    dir_close(dir) ;
    stralloc_free(&contents) ;
    if (wasnull) stralloc_free(modifs) ; else modifs->len = modifbase ;
    errno = e ;
    return -1 ;
  }
}

int main (int argc, char const *const *argv)
{
  stralloc modifs = STRALLOC_ZERO ;
  subgetopt l = SUBGETOPT_ZERO ;
  int insist = 1 ;
  PROG = "justc-envdir" ;
  for (;;)
  {
    int opt = subgetopt_r(argc, argv, "Ii", &l) ;
    if (opt == -1) break ;
    switch (opt)
    {
      case 'I' : insist = 0 ; break ;
      case 'i' : insist = 1 ; break ;
      default : strerr_dieusage(100, USAGE) ;
    }
  }
  argc -= l.ind ; argv += l.ind ;
  if (argc < 2) strerr_dieusage(100, USAGE) ;
  if ((justc_envdir(*argv++, &modifs) < 0) && (insist || (errno != ENOENT)))
    strerr_diefu1sys(111, "justc_envdir") ;
  xmexec_m(argv, modifs.s, modifs.len) ;
}
