# gitall
Run git commands on all local repos under a particular directory ($HOME for 
now). As the following will likely indicate, this project serves only to 
serve a personal need and to provide some C programming practice.

## DISCLAIMER
The author accepts no responsibility for the consequences of running this 
program. You run it entirely at your own risk.

## Installation
First, `cp config.h.tmpl config.h` and edit the latter. At the moment it 
contains a single macro `GITIGNORE`, which yields the path to a file 
containing patterns, supplied to `fnmatch`, for local git repos to be 
ignored by `gitall`. Have `GITIGNORE` yield "" if there's nothing you wish 
to ignore. (Having it yield garbage shouldn't cause a problem.)

Then, `make install [DESTDIR=<destdir>]`. If the optional bit in `[...]` is 
omitted, the executable will be installed by default in `~/.local/bin`; 
otherwise, it will be installed at whatever path you supply in `<destdir>`.

## Usage
`gitall <cmds>`\
where `<cmds>` stands for whatever you would normally supply to `git` in a local repo, e.g `status --porcelain`.

## Behavior
For the moment the dynamic memory allocation in `gitall` is very 
rudimentary:

-  Local git repos with exceedingly long file paths may cause problems.

-  If the output of the `git` command run in a particular repo is 
   exceedingly longer than approx. 4K, then it will be cut off in writing 
   to `stdout`.

Additionally, I have no idea whether this program behaves as expected on a 
system with something other than the GNU C Standard Library.
