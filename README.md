# Assignment 1: sort

Implementation is in `src/sort.cpp`

```bash
make opt
bin/sort $BIG_RANDOM_FILE $OUTFILE 1024

# if it crashes, delete temp files
rm -f externalsort-*
```

Some notes:

- works best with ~ 1GiB of available memory
- no verify (yet)


---

---

# db

This will be a simple database, built in the class _Datenbanksysteme und moderne CPU-Architekturen_.

## Usage

Prerequisites: _build-essential_ & _clang_.

### Building the program

    make opt

**Note:** running _make_ with the default target will produce an unoptimized binary

### Building a debug version and starting it with gdb

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
