# Assignment 3: Slotted Pages

(Hopefully complete) build requisites, example for Debian-ish systems:
```bash
apt-get install libgtest-dev build-essential clang
```

- Implementation is in `src/spsegment.{cpp, h}`
- Unittests are in `test/spsegment.cpp`
- `SPSegment::insert` for some reason does not work with big records -- the SlottedTest will not work.
- `insert` is currently only good for filling up pages, `remove()`d slots will not be reclaimed
- My random approach in `BufferManager::free` proves to be quite slow when doing a lot of work
- No FSI (yet). This makes the slowness even worse
- No `SPSegment::update` until insert works correctly
- I left out Exercise 1, Metadata/schema segment. Didn't have the time.

## Running the unittests

```bash
make test
```

This builds with `-O2`, because the unittests take quite a lot of time without.  
Some of them are disabled (prefixed with "DISABLED_") to mitigate this a bit.

## Running unittests with debug build

Debug builds do some sanity assertions and a lot more output.  
Helpful for valgrinding and debugging.

```bash
make debug
bin/test_db
```

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

Prerequisites: _build-essential_, _clang_ and _googletest_.

```bash
apt-get install libgtest-dev build-essential clang
```

If your distro does not have googletest as a package, unzip the source of [googletest][gtest] into `/usr/src/gtest`. If you chose a different path, open up the [Makefile][makefile] and set the correct path in the variable `GTESTDIR`.

[gtest]: https://code.google.com/p/googletest/downloads/list
[makefile]: https://bitbucket.org/cfstras/db/src/master/Makefile

### Building the program

    make opt

### Building a debug version + Debugging test binary

    make debug

### Running some tests

    make test

### Achieving happiness

    make happiness

## License

Beerware.
