import sys
import random
import argparse
from math import ceil


# Psuedo random number generator, shouldn't repeat ever really
def lcg(modulus: int, a: int, c: int, seed: int):
    """Linear congruential generator."""
    while True:
        seed = (a * seed + c) % modulus
        yield seed


def main(args):
    # Get num_entries
    num_entries = args.num_entries
    if num_entries < 0:
        print("num_entries must be >= 0")
        return
    
    # Initialize generator and space to store generated things
    gen_object = lcg(pow(2,31) + 1, 75, 74, 1)
    entries = [] 
    map_of_entries = {} # Have to have 2 data structures
                        # b/c Python3 automatically sorts dicts
    for i in gen_object:
        if i in map_of_entries:
            print("ERROR: You have undesired repeats due to undefined behavior in the random number generator. To get rid of this, modify the lcg() function parameters.")
            print("Some good idea of parameter values can be found here: https://en.wikipedia.org/wiki/Linear_congruential_generator")
            exit
        else:
            map_of_entries[i]  = len(entries)
        tup = (i, bytearray(random.getrandbits(8) for _ in range(96))) # 96 random bytes
        entries.append(tup)
        if(len(entries) == num_entries): # End when we hit the desired number of entries
            break      
    
    if(args.repeats):
        print(f"Adding {ceil(num_entries / 1000)} repeats.") #Chose 1000 arbitrarily
        rc = random.sample(entries, k=int(2 * ceil(num_entries / 1000)))
        for i in range(2 * ceil(num_entries / 1000)):
            if i % 2 == 0:
                entries[map_of_entries[rc[i][0]]] = rc[i+1] #replace a value with another
    
    fpi = args.input_file # Input file
    fpo = args.output_file # Output file
    
    sorted_things = sorted(entries) # Sorted list to put in output file

    with open(fpi, "wb") as ip, open(fpo, "wb") as op:
        for i in range(num_entries): # Write all entries
            ip.write(entries[i][0].to_bytes(4, sys.byteorder))
            ip.write(entries[i][1])
            op.write(sorted_things[i][0].to_bytes(4, sys.byteorder))
            op.write(sorted_things[i][1])

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('num_entries',
                        help='Number of entries to generate',
                        type=int)
    parser.add_argument('input_file',
                        help='Path to input file you wish to populate',
                        type=str)
    parser.add_argument('output_file',
                        help='Path to output file you wish to populate',
                        type=str)
    parser.add_argument('--r',
                        dest='repeats', 
                        action='store_const',
                        const=True,
                        default=False,
                        help='''Indicate if you want repeats. For grading purposes, repeats can be sorted into any order. 
                            Adding repeats slows the test case generator, only do it for tests with a small number of entries''',)
    main(parser.parse_args())
