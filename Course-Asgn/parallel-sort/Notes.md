# P3a Notes

## Tests
Tests are in `~cs537-1/tests/p3a`. Run as
```
$ ~cs537-1/tests/p3a/test-psort.sh
```

A Python script in there generates more test cases. This script also generates a solution in addition to the input file. Use this to test performance with different file sizes. Run Python script:
```
$ python3 /path/to/gen_tests.py num_entries input_file output_file
```