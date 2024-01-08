# Parallel Sort 

A multi-threaded parallel merge sort algorithm. 
* A new thread is spawned for one of the two subarrays.
* Merge step is done in parallel with two threads each merging about half of the subarrays. 
* For small subarrays, serial procedures are used instead of parallel.

## Run 

For testing, 100-byte elements with 4-byte keys are used.

Generate input and expected output files
```sh
python3 gen_tests.py num_entries input_file expected_output_file
```

Run
```
make
./main input_file output_file algorithm 
```