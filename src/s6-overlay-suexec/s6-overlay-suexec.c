/* ISC license. */

/* for setresuid/setresgid */
#include <skalibs/nonposix.h>

#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include <skalibs/uint64.h>
#include <skalibs/types.h>
#include <skalibs/strerr.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>
#include <skalibs/cspawn.h>
#include <skalibs/djbunix.h>

#include <execline/execline.h>

#define USAGE "s6-overlay-suexec { root_block... } normal_init..."
#define addenv(k, v) if (!env_mspawn(k, v)) strerr_diefu1sys(111, "modify environment")

int main (int argc, char const **argv)
{
  pid_t pid ;
  uid_t euid = geteuid(), uid = getuid() ;
  gid_t egid = getegid(), gid = getgid() ;
  int argc1, wstat ;
  char fmt[UINT64_FMT] ;
  PROG = "s6-overlay-suexec" ;

  if (!--argc) strerr_dieusage(100, USAGE) ;
  if (getpid() != 1) strerr_dief1x(100, "can only run as pid 1") ;
  if (euid)
  {
    if (euid != uid)
    {
      char fmte[UID_FMT] ;
      fmt[uid_fmt(fmt, uid)] = 0 ;
      fmte[uid_fmt(fmte, euid)] = 0 ;
      strerr_warnw7x("real uid is ", fmt, " while effective uid is ", fmte, "; setting everything to ", fmt, "; check s6-overlay-suexec permissions") ;
    }
    if (setresgid(gid, gid, gid) == -1) strerr_diefu1sys(111, "setresgid") ;
    if (setresuid(uid, uid, uid) == -1) strerr_diefu1sys(111, "setresuid") ;
  }
  else
  {
    if (gid) if (setgid(0) == -1) strerr_diefu4sys(111, "set", "g", "id to ", "root") ;
    if (uid) if (setuid(0) == -1) strerr_diefu4sys(111, "set", "u", "id to ", "root") ;
  }
  argc1 = el_semicolon(++argv) ;
  if (!argv[0]) strerr_dief1x(100, "empty block") ;
  if (argc1 >= argc) strerr_dief1x(100, "unterminated block") ;
  if (!argv[argc1 + 1]) strerr_dief1x(100, "empty remainder") ;
  argv[argc1] = 0 ;

  addenv("GIDLIST", 0) ;
  fmt[gid_fmt(fmt, gid)] = 0 ;
  addenv("GID", fmt) ;
  fmt[gid_fmt(fmt, egid)] = 0 ;
  addenv("EGID", fmt) ;
  fmt[uid_fmt(fmt, uid)] = 0 ;
  addenv("UID", fmt) ;
  fmt[uid_fmt(fmt, euid)] = 0 ;
  addenv("EUID", fmt) ;
  {
    struct passwd *pw = getpwuid(uid) ;
    addenv("USER", pw ? pw->pw_name : "") ;
    pw = getpwuid(euid) ;
    addenv("EUSER", pw ? pw->pw_name : "") ;
  }
  {
    struct group *gr = getgrgid(gid) ;
    addenv("GROUP", gr ? gr->gr_name : "") ;
    gr = getgrgid(egid) ;
    addenv("EGROUP", gr ? gr->gr_name : "") ;
  }

  pid = xmspawn(argv, 0, 0, 0) ;
  if (wait_pid(pid, &wstat) == -1) strerr_diefu1sys(111, "wait_pid") ;
  if (WIFSIGNALED(wstat))
  {
    fmt[uint_fmt(fmt, WTERMSIG(wstat))] = 0 ;
    strerr_dief2x(128 + WTERMSIG(wstat), "child crashed with signal ", fmt) ;
  }
  else if (WEXITSTATUS(wstat))
  {
    fmt[uint_fmt(fmt, WEXITSTATUS(wstat))] = 0 ;
    strerr_dief2x(wait_estatus(wstat), "child failed with exit code ", fmt) ;
  }

  if (!euid)
  {
    if (gid) if (setgid(gid) == -1) strerr_diefu3sys(111, "set", "g", "id to user") ;
    if (uid) if (setuid(uid) == -1) strerr_diefu3sys(111, "set", "u", "id to user") ;
  }
  xexec(argv + argc1 + 1) ;
}
