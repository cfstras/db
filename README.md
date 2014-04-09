# db

This is a simple database, built in the class _Datenbanksysteme und moderne CPU-Architekturen_.

## Usage

Prerequisites: build-essential & clang.

### Building the program

    #TODO: not optimizing yet
    make

### Building a non-optimized, debug version

    make debug

### Running some tests

First, get the latest source of googletest [here][gtest]. Unzip it somewhere, preferably in `/usr/src/gtest`. If you chose a different path, open up the [Makefile][makefile] and set the correct path in the variable `GTESTDIR`.  
Then:

    make test

[gtest]: https://code.google.com/p/googletest/downloads/list
[makefile]: https://bitbucket.org/cfstras/db/src/master/Makefile

### Achieving happiness

    make happiness

## License

Beerware.
