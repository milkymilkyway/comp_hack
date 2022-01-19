#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import datetime
import os
import sys
import subprocess
import random
import time
import junit_xml
import optparse
import tempfile
import xml.dom.minidom

from click import option

TEST_TIMEOUT = 60  # timeout in 1 min


class bcolors:
    # https://stackoverflow.com/questions/287871/how-to-print-colored-text-to-the-terminal
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def parse_command_line():
    parser = optparse.OptionParser()
    parser.add_option("-m", "--manager", dest="manager",
                      help="Path to the comp_manager application", metavar="FILE")
    parser.add_option("-c", "--client", dest="client",
                      help="Path to the comp_client application", metavar="FILE")
    parser.add_option("-s", "--server-data", dest="testing",
                      help="Path to the testing server data directory", metavar="DIR")
    parser.add_option("-r", "--root-dir", dest="root",
                      help="Root directory for the tests", metavar="ROOT")
    parser.add_option("-o", "--output", dest="output",
                      help="Output path for the JUnit XML result file", metavar="OUTPUT")

    (options, args) = parser.parse_args()

    if not options.manager:
        parser.error("Path to the comp_manager application must be specified!")

    if not options.client:
        parser.error("Path to the comp_client application must be specified!")

    if not options.testing:
        parser.error(
            "Path to the testing server data directory must be specified!")

    if not options.root:
        options.root = os.path.dirname(os.path.realpath(__file__))

    options.manager = os.path.abspath(options.manager)
    options.client = os.path.abspath(options.client)
    options.testing = os.path.abspath(options.testing)
    options.root = os.path.abspath(options.root)

    if not os.path.isfile(options.manager) or not os.access(options.manager, os.X_OK):
        parser.error(
            "Path to comp_manager is incorrect of tool is not executable!")

    if not os.path.isfile(options.client) or not os.access(options.client, os.X_OK):
        parser.error(
            "Path to comp_client is incorrect of tool is not executable!")

    if not os.path.isdir(options.testing):
        parser.error(
            "Testing server data directory does not exist or is not a directory!")

    if not os.path.isfile(os.path.join(options.testing, "programs.xml")):
        parser.error(
            "Testing server data directory does not contain a programs.xml!")

    if not os.path.isdir(options.root):
        parser.error(
            "Root directory for tests does not exist or is not a directory!")

    if len(args) < 1:
        parser.error("At least one test or test directory must be specified!")

    # print("Manager:     {}".format(options.manager))
    # print("Client:      {}".format(options.client))
    # print("Server Data: {}".format(options.testing))
    # print("Tests Dir:   {}".format(options.root))

    return (options, args)


def parse_program_xml(options, test_path):
    dom = xml.dom.minidom.parse(os.path.join(options.testing, "programs.xml"))
    root = dom.documentElement

    path = dom.createElement("path")
    path.appendChild(dom.createTextNode(options.client))

    arg = dom.createElement("arg")
    arg.appendChild(dom.createTextNode(test_path))

    program = dom.createElement("program")
    program.setAttribute("timeout", "0")
    program.setAttribute("restart", "false")
    program.setAttribute("output", "true")
    program.setAttribute("notify", "false")
    program.setAttribute("stop_on_exit", "true")
    program.appendChild(path)
    program.appendChild(arg)

    root.appendChild(program)

    file = tempfile.NamedTemporaryFile(
        mode="w", prefix="comp_manager_programs_", suffix=".xml")
    dom.writexml(file)
    file.flush()

    return file


def process_test(tests, root, path):
    file = path[len(root)+1:]

    group = os.path.dirname(file)
    test = os.path.splitext(os.path.basename(file))[0]

    if not group in tests:
        tests[group] = []

    if test in tests[group]:
        print("Duplicate test ignored: {}".format(test), file=sys.stderr)
        return 0
    else:
        tests[group].append(test)
        return 1


def process_test_list(options, args):
    errors = 0
    test_count = 0
    tests = {}

    (skip_groups, skip_tests) = parse_file_list(
        os.path.join(options.root, "skip.lst"))
    (required_groups, required_tests) = parse_file_list(
        os.path.join(options.root, "required.lst"))

    for arg in args:
        if os.path.isfile(arg):
            file = os.path.abspath(arg)

            if len(file) < len(options.root) or file[0:len(options.root)] != options.root:
                print("File is not contained in the test directory: {}".format(
                    file), file=sys.stderr)
                errors += 1
            elif os.path.splitext(file)[1] != ".nut":
                print("File is not a Squirrel script: {}".format(
                    file), file=sys.stderr)
                errors += 1

            test_count + + process_test(tests, options.root, file)
        elif os.path.isdir(arg):
            dir = os.path.abspath(arg)

            if len(dir) < len(options.root) or dir[0:len(options.root)] != options.root:
                print("Directory is not contained in the test directory: {}".format(
                    dir), file=sys.stderr)
                errors += 1

            for root, subdirs, files in os.walk(dir):
                for file in files:
                    if file.endswith(".nut"):
                        test_count += process_test(tests,
                                                   options.root, os.path.join(root, file))

        elif not os.path.exists(arg):
            print("File or directory does not exist: {}".format(
                arg), file=sys.stderr)
            errors += 1
        else:
            print("Argument is not a file or directory: {}".format(
                arg), file=sys.stderr)
            errors += 1

    if errors > 0:
        sys.exit(-1)

    print("Collected {} test(s) in {} group(s)".format(test_count, len(tests)))
    print("")

    return (test_count, tests, skip_groups, skip_tests, required_groups, required_tests)


def parse_file_list(path):
    try:
        files = []
        dirs = []

        with open(path, "r") as file:
            for line in file.readlines():
                line = line.strip()

                if len(line) <= 0 or line.startswith("#"):
                    continue

                if line.endswith("/"):
                    if not line in dirs:
                        dirs.append(line[0:-1])
                else:
                    line = os.path.splitext(line)[0]

                    if not line in files:
                        files.append(line)

        return (dirs, files)
    except FileNotFoundError:
        return []


def run_test(options, group, test, test_count, total_tests, test_cases):
    print("Starting test: {} ({}/{})".format(os.path.join(group, test),
          test_count, total_tests))
    print("-" * 80)

    programs = parse_program_xml(options, os.path.join(
        options.root, group, test + ".nut"))

    output = None
    status = False
    did_timeout = False

    tic = time.perf_counter()

    try:
        p = subprocess.Popen([options.manager, programs.name], stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, cwd=options.testing, start_new_session=True)
        p.wait(timeout=TEST_TIMEOUT)
        output, _ = p.communicate()
        status = 0 == p.returncode
    except subprocess.CalledProcessError as e:
        output, _ = p.communicate()
        status = False
    except subprocess.TimeoutExpired as e:
        try:
            os.killpg(os.getpgid(p.pid), subprocess.signal.SIGKILL)
            p.wait()
        except ProcessLookupError:
            pass

        output, _ = p.communicate()
        status = False
        did_timeout = True

    output = output.decode("utf-8", errors="ignore")
    toc = time.perf_counter()

    test_case = junit_xml.TestCase(test, os.path.join(
        group, test).replace("/", "."), toc - tic, output)

    if not status:
        test_case.add_failure_info(
            "Test case has failed. See log for details.")

    test_cases.append(test_case)

    print(output)

    if did_timeout:
        print(
            f"{bcolors.FAIL}TIMEOUT OF {TEST_TIMEOUT} SECONDS HAS BEEN REACHED {bcolors.ENDC}")

    if status:
        print(f"{bcolors.OKGREEN}PASSED{bcolors.ENDC}")
    else:
        print(f"{bcolors.FAIL}FAILED{bcolors.ENDC}")

    return status


def run_tests(options, test_count, tests, skip_groups, skip_tests, required_groups, required_tests):
    failed_count = 0
    pass_count = 0
    skip_count = 0
    status = True

    group_count = 1
    total_groups = len(tests)
    test_suites = []

    for group, tests_in_group in tests.items():
        group_name = group
        if len(group_name) == 0:
            group_name = "<empty>"

        random.shuffle(tests_in_group)

        test_count = 1
        total_tests = len(tests_in_group)
        test_cases = []

        print("=" * 80)
        print("Starting {} test(s) in group: {} ({}/{})".format(total_tests,
              group_name, group_count, total_groups))
        print("=" * 80)
        print("")

        for test in tests_in_group:
            test_path = os.path.join(group, test)

            if group in skip_groups or test_path in skip_tests:
                skip_count += 1

                print("Skipped test: {} ({}/{})".format(os.path.join(group,
                      test), test_count, total_tests))
                print("-" * 80)
                print(f"{bcolors.WARNING}SKIPPED{bcolors.ENDC}")
                print("")

                test_case = junit_xml.TestCase(test, os.path.join(
                    group, test).replace("/", "."))
                test_case.add_skipped_info("Test case has been skipped.")
                test_cases.append(test_case)
            elif run_test(options, group, test, test_count, total_tests, test_cases):
                pass_count += 1
                print("")
            else:
                failed_count += 1

                if group in required_groups or test_path in required_tests:
                    print(f"{bcolors.FAIL}REQUIRED TEST HAS FAILED{bcolors.ENDC}")
                    print("")

                    status = False

            test_count += 1

        test_suites.append(junit_xml.TestSuite(
            group, test_cases, timestamp=datetime.datetime.now()))

        group_count += 1

    print("=" * 80)
    print("Finished {} tests in {} groups: {} passed, {} failed, {} skipped".format(
        test_count, total_groups, pass_count, failed_count, skip_count))
    print("=" * 80)

    if options.output:
        with open(options.output, 'w') as f:
            junit_xml.TestSuite.to_file(f, test_suites, prettyprint=True)

    return status


if __name__ == '__main__':
    (options, args) = parse_command_line()
    (test_count, tests, skip_groups, skip_tests, required_groups,
     required_tests) = process_test_list(options, args)

    if run_tests(options, test_count, tests, skip_groups, skip_tests, required_groups, required_tests):
        sys.exit(0)
    else:
        sys.exit(-1)
