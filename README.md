# s6-overlay-helpers

Small utilities specifically written for
[s6-overlay](https://github.com/just-containers/s6-overlay).

s6-overlay-helpers centralizes all the additional C programs
and libraries that are not part of any [skarnet.org](https://skarnet.org/software/)
package but are used specifically in [s6-overlay](https://github.com/just-containers/s6-overlay).

## Installation

See the INSTALL file.

## License

ISC; see the COPYING file.

# s6-overlay-suexec

This is a small program that can only be run as pid 1; it ensures that
a given block of code is always run as root, even if the container is run
with the USER directive. After the root block exits, if the USER directive
has been given, privileges are dropped again and the rest of the execution
proceeds as a normal user.

This is used in [s6-overlay](https://github.com/just-containers/s6-overlay)
to make sure that a portion of the filesystem hierarchy always exists and
is owned by the current user.

This binary needs the *suid* bit enabled so it can gain privileges to
execute its argument as root. Since it can only be run as part of the pid 1
process chain, it does not endanger the security of the system; nevertheless,
please audit the code until you have full confidence that it is secure.

## Usage

In an execline script: `s6-overlay-suexec { root_block... } normal_init...`

On the command line: `s6-overlay-suexec root_block... '' normal_init...`

# s6-overlay-stat

A simple utility that prints the uid and gid (and user and group, if available
in `/etc/passwd` and `/etc/group`) of a file given as argument. The output
is suitable for setting the `uid`, `gid`, `user`, `group` and `perms` variables for
the rest of the script.

## Usage

In an execline script: `pipeline { s6-overlay-stat file } envfile - prog...`

In a shell script: `` eval `s6-overlay-stat file` ``
