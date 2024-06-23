import sys
import csv
import operator

""" argv = sys.argv
reader = csv.reader(open(argv[1]), delimiter=";")
sortedList = sorted(reader, key=operator.itemgetter(1))
writer = csv.writer("test_sorted.csv") """

if len(sys.argv) != 3:
    print("Usage: python sort_csv.py <input_file> <sort_by column>")
    print("Given args: ", sys.argv)
    sys.exit(1)

in_file_name = sys.argv[1]
sort_by = sys.argv[2]

with open(in_file_name, 'r') as in_file:
    reader = csv.reader(in_file, delimiter=";")
    #header = next(reader)
    data = sorted(reader, key=lambda row: float(row[1]))

with open('s_' + in_file_name, 'w', newline='') as out_file:
    out_writer = csv.writer(out_file, delimiter=";")
    #out_writer.writerow(header)
    out_writer.writerows(data)

print("Sorted file saved as: ", 's_' + in_file_name)
