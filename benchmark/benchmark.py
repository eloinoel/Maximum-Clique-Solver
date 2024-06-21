import argparse
import os
import re
import time
import sys

import subprocess


def run_command_with_limit(cmd, input_file, timeout):
    """
    Run a command with a time limit and redirecting stdin from a file.

    cmd: string, the command to run
    input_file: string, path to the file for stdin redirection
    timeout: int, time limit in seconds

    Returns: tuple containing return code, execution time, stdout, stderr,
             and a boolean indicating if the process was terminated due to a timeout
    """

    try:
        start_time = time.time()

        # If cmd is a string, split it into a list for subprocess.run
        if isinstance(cmd, str):
            cmd = cmd.split()

        with open(input_file, 'r') as f:
            completed_process = subprocess.run(cmd, stdin=f, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                               timeout=timeout)


        stdout = completed_process.stdout.decode('utf-8')
        stderr = completed_process.stderr.decode('utf-8')

        return_code = completed_process.returncode
        end_time = time.time()
        was_timeout = False

    except subprocess.TimeoutExpired:
        # This block will be entered if the command times out
        end_time = time.time()
        stdout = ''
        stderr = ''
        return_code = ''
        was_timeout = True

    except subprocess.CalledProcessError as e:
        # This block will be entered if the command fails
        end_time = time.time()
        stdout = e.stdout.decode('utf-8') if e.stdout else ''
        stderr = e.stderr.decode('utf-8') if e.stderr else ''
        return_code = e.returncode
        was_timeout = False

    except Exception as e:
        # This block will be entered if an unexpected exception occurs
        end_time = time.time()
        stdout = ''
        stderr = str(e)
        return_code = -1
        was_timeout = False

    return return_code, end_time - start_time, stdout, stderr, was_timeout


def extract_starting_numerical_prefix(filename):
    number = ''.join(filter(str.isdigit, filename))
    return int(number) if number else 0


def get_instances(in_dir):
    groups = {}

    sorted_files = sorted(os.listdir(in_dir))
    sorted_files = [f for f in sorted_files if f.endswith(".dimacs")]

    return sorted_files

class CollectDataAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs='?', const='data.csv', **kwargs):
        super(CollectDataAction, self).__init__(option_strings, dest, nargs=nargs, const=const, **kwargs)
        self.default = {'value': const, 'flag': False}

    def __call__(self, parser, namespace, values, option_string=None):
        if values is None:
            values = self.const
        setattr(namespace, self.dest, {'value': values, 'flag': True})

def create_parser():
    parser = argparse.ArgumentParser(description='Process some executables.')

    parser.add_argument('executable', type=str, help='Command to run your solver')
    parser.add_argument('--time_limit', type=int, default=60, help='Time limit [sec] (default: 60)')
    parser.add_argument('--max_time_limit_exceeded', type=int, default=10,
                                   help='Max time limit exceeded (default: 10)')
    parser.add_argument('--collect_data', action=CollectDataAction, default='data.csv', help='Store output of solver in csv format (default: \'data.csv\')')
    return parser

dir = "logs"
def write_data_to_file(filename, data):
    os.makedirs(dir, exist_ok=True)
    filepath = os.path.join(dir, filename)
    if not os.path.exists(filepath):
        with open(filepath, 'w') as result:
            result.write(data)
    else:
        with open(filepath, 'a') as result:
            result.write(data)

def clear_file(filename):
    os.makedirs(dir, exist_ok=True)
    filepath = os.path.join(dir, filename)
    if os.path.exists(filepath):
        with open(filepath, 'w') as result:
            result.write(' ')

def main():
    parser = create_parser()

    args = parser.parse_args()

    time_limit = args.time_limit
    max_time_limit_exceeded = args.max_time_limit_exceeded

    collect_data_flag = args.collect_data['flag']
    collect_data_file_name = args.collect_data['value']
    if collect_data_flag:
        clear_file(collect_data_file_name)

    checker_file = "verify.py"

    if not collect_data_flag:
        print("file,status,time,return,stderr")

    found_error = False

    score = 0

    for dataset in ["random", "datacenter", "bio", "dimacs", "pace"]:
        dataset_score = 0
        if found_error:
            break
        print("Crunching dataset", dataset, file=sys.stderr)

        in_dir = os.path.join("data", dataset)

        tles = 0
        for f in get_instances(in_dir):
            in_file = os.path.join(in_dir, f)
            points = 0

            # run executable on instance
            return_code, time, stdout, stderr, was_timeout = run_command_with_limit(args.executable, in_file, time_limit)
            time = "{:.3f}".format(time)

            if not was_timeout and return_code == 0:
                out_file = in_file.replace(".dimacs", ".reference")

                assert(os.path.exists(checker_file))
                if os.path.exists(checker_file):
                    if collect_data_flag:
                        write_data_to_file(collect_data_file_name, stdout)
                    result = subprocess.run(["python3", checker_file, in_file, "user_out.txt", out_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    stdout = result.stdout.decode('utf-8')
                    if "OK" in stdout:
                        status = "OK"
                    else:
                        status = "Wrong"
                        stderr += "\n" + stdout + "\n" + stderr
            elif was_timeout:
                status = "timelimit"
                time = ''
            else:
                stderr += "\nNon zero exit code"
                status = "Wrong"

            stderr = re.sub(r"\n", r"[\\n]", stderr)
            print(f"{f},{status},{time},{return_code},{stderr}", flush=True)

            #if status == "Wrong" and not args.collect_data:
            if status == "Wrong" and not collect_data_flag:
                found_error = True
                break
            elif status == "timelimit":
                tles += 1
                if tles >= max_time_limit_exceeded:
                    break
            else:
                score += 1
                dataset_score += 1
        print("Dataset score =", dataset_score, file=sys.stderr)
    print("Final score =", score, file=sys.stderr)


if __name__ == '__main__':
    main()
