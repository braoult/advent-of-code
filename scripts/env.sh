# to be sourced, it allows to run C programs with libraries in ../lib.
export LD_LIBRARY_PATH=../lib

# to get the same output than the default (Makefile) one.
# usage:
# TIME command < inputfile
alias TIME='\time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"'

# Get (move) ~/Downloads/Day* file to README.html in current directory, then
# run "make org"
org() {
    mv ~/Downloads/Day* README.html
    make org
    rm README.html
}
