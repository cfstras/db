# Assignment 1: sort

- To build, you need the googletest source located in `/usr/src/gtest` 
- Implementation is in `src/sort.cpp`
- Unittests are in `test/sort.cpp`
- SortTest.SortHuge does the 5GiB test. 

## Sorting your own file

```bash
make opt
bin/sort $BIG_RANDOM_FILE $OUTFILE 1024
```

This builds with `-O3`.

## Running the unittests

```bash
make test
```

This builds with `-O2`, because the huge unittest takes quite a lot of time without.

## Cleaning up

```bash
make clean
```


---

---

---

# db

This will be a simple database, built in the class _Datenbanksysteme und moderne CPU-Architekturen_.

## Usage

Prerequisites: _build-essential_ & _clang_.

Also, get the latest source of [googletest][gtest]. Unzip it somewhere, preferably in `/usr/src/gtest`. If you chose a different path, open up the [Makefile][makefile] and set the correct path in the variable `GTESTDIR`.  

[gtest]: https://code.google.com/p/googletest/downloads/list
[makefile]: https://bitbucket.org/cfstras/db/src/master/Makefile

### Building the program

    make opt

### Building a debug version and starting it with gdb

    make debug

### Running some tests

    make test

### Achieving happiness

    make happiness

## License

Beerware.
