/* ISC license. */

#include <unistd.h>
#include <sys/wait.h>

#include <skalibs/types.h>
#include <skalibs/strerr2.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>
#include <skalibs/djbunix.h>

#include <execline/execline.h>

#define USAGE "s6-overlay-suexec { root_block... } normal_init..."

int main (int argc, char const **argv, char const *const *envp)
{
  pid_t pid ;
  uid_t uid ;
  gid_t gid ;
  int argc1, wstat ;

  PROG = "s6-overlay-suexec" ;

  if (!--argc) strerr_dieusage(100, USAGE) ;
  if (getpid() != 1) strerr_dief1x(100, "can only run as pid 1") ;  
  if (geteuid()) strerr_dief1x(100, "insufficient privileges (is the suid bit set?)") ;

  uid = getuid() ;
  gid = getgid() ;

  argc1 = el_semicolon(++argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  argv[argc1] = 0 ;
  
  {
    size_t len = env_len(envp) ;
    size_t m = 12 ;
    char fmt[UID_FMT + GID_FMT + 10] = "GIDLIST\0GID=" ;
    char const *v[len + 4] ;
    m += gid_fmt(fmt + m, gid) ;
    memcpy(fmt + m, "\0UID=", 5) ; m += 5 ;
    m += uid_fmt(fmt + m, uid) ;
    fmt[m++] = 0 ;
    if (!env_mergen(v, len + 4, envp, len, fmt, m, 3))
      strerr_diefu1sys(111, "merge environment") ;
    if (gid) if (setgid(0) == -1) strerr_diefu2sys(111, "setgid to ", "root") ;
    if (uid) if (setuid(0) == -1) strerr_diefu2sys(111, "setuid to ", "root") ;
    pid = el_spawn0(argv[0], argv, v) ;
    if (!pid) strerr_diefu2sys(111, "spawn ", argv[0]) ;
  }

  if (wait_pid(pid, &wstat) == -1) strerr_diefu1sys(111, "wait_pid") ;
  if (WIFSIGNALED(wstat))
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, WTERMSIG(wstat))] = 0 ;
    strerr_dief2x(128 + WTERMSIG(wstat), "child crashed with signal ", fmt) ;
  }
  if (WEXITSTATUS(wstat))
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, WEXITSTATUS(wstat))] = 0 ;
    strerr_dief2x(wait_estatus(wstat), "child failed wth exit code ", fmt) ;
  }

  if (gid) if (setgid(gid) == -1) strerr_diefu2sys(111, "setgid to ", "user") ;
  if (uid) if (setuid(uid) == -1) strerr_diefu2sys(111, "setuid to ", "user") ;

  xexec_e(argv + argc1 + 1, envp) ;
}
