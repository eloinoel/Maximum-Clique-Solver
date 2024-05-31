import argparse
import sys
import re


def create_parser():
    parser = argparse.ArgumentParser(description='Check max-clique solver output')

    parser.add_argument('input_file', type=str)
    parser.add_argument('user_output_file', type=str)
    parser.add_argument('reference_solution_file', type=str)

    return parser

parser = create_parser()
args = parser.parse_args()

def read_solution_size(solution_file):
    with open(solution_file, 'r') as f:
        for line in f:
            if line.startswith("#"):
                continue
            words = line.split()
            sol_size = int(words[0])
            return sol_size
        raise ValueError

def read_graph(file):
    G = dict()
    G["V"] = set()
    G["E"] = set()
    G["neighs"] = dict()

    with open(file, 'r') as f:
        for line in f.readlines():
            line = line.split("#")[0].strip()
            if len(line) == 0:
                continue

            u = line.split()[0]
            v = line.split()[1]
            G["V"].add(u)
            G["V"].add(v)
            G["E"].add((u, v))

            if u not in G["neighs"]:
                G["neighs"][u] = set()
            if v not in G["neighs"]:
                G["neighs"][v] = set()

            G["neighs"][u].add(v)
            G["neighs"][v].add(u)

    return G


def read_solution(file):
    C = []
    with open(file, 'r') as f:
        for line in f.readlines():
            line = line.split("#")[0].strip()
            if line == '':
                continue  # Skip empty lines

            C.append(line)
    return C

reference_sol = read_solution_size(args.reference_solution_file)
Cand = read_solution(args.user_output_file)
G = read_graph(args.input_file)

errors = set()

if len(set(Cand)) != len(Cand):
    errors.add("duplicate vertices")

C = set()
for v in Cand:
    if v not in G["V"]:
        errors.add("unknown vertices")
    else:
        C.add(v)

if reference_sol is not None:
    if len(C) < reference_sol:
        errors.add("solution is not maximum")

for v in C:
    found = 0
    for u in G["neighs"][v]:
        if u in C:
            found += 1
    if found + 1 != len(C):
        errors.add("not a clique")
        break

errors = list(sorted(errors))

if len(errors) > 0:
    print("WRONG\n"+",".join(errors))
    sys.exit(1)
else:
    print("OK")
