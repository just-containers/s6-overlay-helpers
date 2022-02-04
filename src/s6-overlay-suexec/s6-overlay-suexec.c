/* ISC license. */

/* for setresuid/setresgid */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include <skalibs/types.h>
#include <skalibs/strerr2.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>
#include <skalibs/djbunix.h>

#include <execline/execline.h>

#define USAGE "s6-overlay-suexec { root_block... } normal_init..."

#define MAXUSERLEN 64
#define MAXGROUPLEN 64
#define MAXLEN (UID_FMT + GID_FMT + MAXUSERLEN + MAXGROUPLEN + 63)

int main (int argc, char const **argv, char const *const *envp)
{
  pid_t pid ;
  uid_t euid = geteuid(), uid = getuid() ;
  gid_t gid = getgid() ;
  int argc1, wstat ;
  size_t len = env_len(envp) ;
  size_t m = 0 ;
  unsigned int n = 0 ;
  char fmt[MAXLEN] = "GIDLIST\0GID=" ;
  PROG = "s6-overlay-suexec" ;

  if (!--argc) strerr_dieusage(100, USAGE) ;
  if (getpid() != 1) strerr_dief1x(100, "can only run as pid 1") ;
  if (euid)
  {
    strerr_warnwu1x("gain root privileges (is the suid bit set?)") ;
    if (setresgid(gid, gid, gid) == -1) strerr_diefu1sys(111, "setresgid") ;
    if (setresuid(uid, uid, uid) == -1) strerr_diefu1sys(111, "setresuid") ;
  }

  argc1 = el_semicolon(++argv) ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  argv[argc1] = 0 ;
  
  {
    char const *v[len + 4] ;
    if (!euid)
    {
      memcpy(fmt + m, "GIDLIST\0GID=", 12) ; m += 12 ;
      m += gid_fmt(fmt + m, gid) ; fmt[m++] = 0 ;
      memcpy(fmt + m, "\0UID=", 5) ; m += 5 ;
      m += uid_fmt(fmt + m, uid) ;
      fmt[m++] = 0 ;
    }
    if (!env_mergen(v, len + 4, envp, len, fmt, m, 3))
      strerr_diefu1sys(111, "merge environment") ;
    if (!euid)
    {
      if (gid) if (setgid(0) == -1) strerr_diefu4sys(111, "set", "g", "id to ", "root") ;
      if (uid) if (setuid(0) == -1) strerr_diefu4sys(111, "set", "u", "id to ", "root") ;
    }
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
    strerr_dief2x(wait_estatus(wstat), "child failed with exit code ", fmt) ;
  }

  m = 0 ;
  if (!euid)
  {
    if (gid) if (setgid(gid) == -1) strerr_diefu4sys(111, "set", "g", "id to ", "user") ;
    if (uid) if (setuid(uid) == -1) strerr_diefu4sys(111, "set", "u", "id to ", "user") ;
    if (gid)
    {
      struct group *gr = getgrgid(gid) ;
      memcpy(fmt + m, "CONTAINER_GID=", 14) ; m += 14 ;
      m += gid_fmt(fmt + m, gid) ;
      fmt[m++] = 0 ; n++ ;
      if (gr)
      {
        size_t rlen = strlen(gr->gr_name)  + 1 ;
        memcpy(fmt + m, "CONTAINER_GROUP=", 16) ; m += 16 ;
        if (rlen > MAXGROUPLEN)
        {
          errno = ENAMETOOLONG ;
          strerr_diefu4sys(111, "fill CONTAINER_", "GROUP", " environment variable with value ", gr->gr_name) ;
        }
        memcpy(fmt + m, gr->gr_name, rlen) ; m += rlen ; n++ ;
      }
    }
    if (uid)
    {
      struct passwd *pw = getpwuid(uid) ;
      memcpy(fmt + m, "CONTAINER_UID=", 14) ; m += 14 ;
      m += uid_fmt(fmt + m, uid) ;
      fmt[m++] = 0 ; n++ ;
      if (pw)
      {
        size_t rlen = strlen(pw->pw_name)  + 1 ;
        memcpy(fmt + m, "CONTAINER_USER=", 15) ; m += 15 ;
        if (rlen > MAXUSERLEN)
        {
          errno = ENAMETOOLONG ;
          strerr_diefu4sys(111, "fill CONTAINER_", "USER", " environment variable with value ", pw->pw_name) ;
        }
        memcpy(fmt + m, pw->pw_name, rlen) ; m += rlen ; n++ ;
      }
    }
  }

  xmexec_en(argv + argc1 + 1, envp, fmt, m, n) ;
}
