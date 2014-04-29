# Assignment 2: buffermanager

(Hopefully complete) build requisites, example for Debian-ish systems:
```bash
apt-get install libgtest-dev build-essential clang
```

- Implementation is in `src/buffer*.cpp`
- Unittests are in `test/buffer*.cpp`
- BufferManager::freePage cannot handle more than _size_ pages being in use (yet)

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
