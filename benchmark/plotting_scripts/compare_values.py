import sys
import csv

if len(sys.argv) != 4:
    print("Usage: python compare_values.py <input_file> <column 1> <column 2>")
    print("Given args: ", sys.argv)
    sys.exit(1)

in_file_name = sys.argv[1]
col1 = int(sys.argv[2])
col2 = int(sys.argv[3])

in_file = open(in_file_name, 'r')
for line in in_file:
    delimeter = ";" if ";" in line else ","
    split = line.split(delimeter)

    val1 = int(split[col1])
    val2 = int(split[col2])

    div = val2 - val1
    print(div, end=", ")