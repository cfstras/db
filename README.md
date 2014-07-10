
# db

This is the ground work for a simple database, built in the class _Datenbanksysteme und moderne CPU-Architekturen_.

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

### Completeness
#### Implemented Operators
    - PrintOperator
    - ProjectionOperator
    - SelectionOperator
    - HashJoinOperator  
    (slow)
    - DummyOperator  
    (reads from vector<vector<string>>)

#### Missing Operators
    - TableScanOperator  
    should be aware of slots etc. and iterate through them
    - BTreeScanOperator  
    should use BTree::next (which, in turn, is missing) :(

## License

`reference/*`: As specified in file headers.  
Everything else: Beerware.
