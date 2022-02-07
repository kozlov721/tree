## Tree

Just a simple implementation of POSIX compliant `tree` command


To build the program, just use the provided makefile.

Usage: [-sadpxUC] [-d DEPTH] [-h] DIRECTORY

OPTIONS:
  -s          Sorts printed files by size instead of by name.
  -a          Prints apparent size instead of the size in blocks.
  -d [DEPTH]  Limits the depth of output, but scans the whole tree.
  -p          options size format to percents instead of the size itself.
  -x          Does not cross filesystem boundaries when performing scan.
  -U          Prints ascii characters instead of unicode ones.
  -C          Disable colors.
  -h          Prints this help and ends the program.
