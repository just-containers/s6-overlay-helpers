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


# justc-envdir

This is a quick and dirty fork of the [s6-envdir](https://skarnet.org/software/s6/s6-envdir.html) program.

Differences between this version and `s6-envdir`:

| `justc-envdir` | `s6-envdir` |
|----------------|-------------|
| Reads the entire file into the environment. | Truncates at 4095 bytes. |
| Allows setting empty environment variables with empty files. | Removes environment variables on empty files. |
| Does not transform `NULL` bytes, terminates variable at first `NULL` byte. | Transforms `NULL` bytes. |

You will need to to take your own precautions around maximum file sizes.

## Usage

`justc-envdir [ -I | -i ] dir prog...`

* justc-envdir reads files in *dir*. For every file *f* in *dir*, that does not begin with a dot and does not contain the `=` character:
    * Add a variable named *f* to the environment (or replace *f* if it already exists) with the contents of the file *f* as value. The file is read verbatim, if the file contains a `NULL`, the value is terminated at that byte.

## Options

* `-i` : strict. If *dir* does not exist, exit 111 with an error message. This is the default.
* `-I` : loose. If *dir* does not exist, exec into *prog* without modifying the environment first.


# s6-overlay-suexec

This is a small program that can only be run as pid 1; it ensures that
a given block of code is always run as root, even if the container is run
with the USER directive. After the root block exits, if the USER directive
has been given, privileges are dropped again and the rest of the execution
proceeds as a normal user.

This is used in [s6-overlay](https://github.com/just-containers/s6-overlay)
to make sure that a portion of the filesystem hierarchy always exists and
is owned by the current user.

This binary needs the _suid_ bit enabled so it can gain privileges to
execute its argument as root. Since it can only be run as part of the pid 1
process chain, it does not endanger the security of the system; nevertheless,
please audit the code until you have full confidence that it is secure.

## Usage

In an execline script: `s6-overlay-suexec { root_block... } normal_init...`

On the command line: `s6-overlay-suexec root_block... '' normal_init...`
