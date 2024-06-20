#!/bin/bash

mkdir build 
cd build
mkdir benchmark
cd benchmark
cmake cmake -DCMAKE_BUILD_TYPE="benchmark" ../..
make

folder_path="../../benchmark/data"
output_file="output.csv"

rm $output_file

#first line in output.csv
line_to_write="Branch and Bound time [s], Bounding time [s], Coloring time [s], k-core time [s], N, M, max degree, min degree, max-k-core, max_clique"
echo "$line_to_write" > "$output_file"

# iterate over all graphs in data directory
for file in "$folder_path"/*; do
    if [ -f "$file" ]; then
        ./mc_solver < $file
        echo -n .
    fi
done
