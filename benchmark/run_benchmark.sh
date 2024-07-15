cd ..

cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

cd benchmark

if [[ $# -eq 0 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver"
fi

if [[ $# -eq 1 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver" --time_limit $1
fi

if [[ $# -eq 2 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver" --time_limit $1 --max_time_limit_exceeded $2
fi