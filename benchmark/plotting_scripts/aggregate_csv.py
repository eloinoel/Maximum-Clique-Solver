import sys
import numpy as np



def main():
    # Using readlines()
    in_file_name = sys.argv[1]
    index = int(sys.argv[2])

    file = open(in_file_name, 'r')
    out_file = open("agg_"+in_file_name, 'w')

    agg_time = np.double(0.0)
    #header
    line = file.readline()
    split = line.split(';')
    out_file.write(split[0]+";"+split[index])

    for file_line in file:
        split = file_line.split(';')
        exec_time = split[index]
        # timeout, not needed atm
        if(exec_time == "-1"):
            #out_file.write("\n")
            continue
        # write test file name in column 0
        try:
            # valid time
            agg_time += np.double(exec_time)
            out_file.write(split[0]+";")
            out_file.write(str(agg_time) + "\n")
        except:
            pass

    file.close()
    out_file.close()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python aggregate_csv.py <input_file> <index over which to aggregate>")
        print("Given args: ", sys.argv)
        sys.exit(1)
    main()