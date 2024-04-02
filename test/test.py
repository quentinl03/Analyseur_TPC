#!/bin/python

import unittest
import logging
import os
import sys
import argparse
from pathlib import Path
from subprocess import run

# Get project's path
PROJECT = Path(__file__).resolve().parents[1]
EXECUTABLE = (PROJECT / "bin" / "tpcc").resolve()

# cd to test directory to make globs easier
os.chdir(PROJECT / "test")

logger = logging.getLogger("Test")

class ColoredFormatter(logging.Formatter):
    """Formats logging messages with colors"""
    color_reset = "\x1b[0m"
    colors = {
        logging.DEBUG: "\x1b[37m",
        logging.INFO: "\x1b[32m",
        logging.WARNING: "\x1b[33m",
        logging.ERROR: "\x1b[31m",
    }
    formatted = "%(message)s"

    def format(self, record) -> str:
        string = self.colors.get(record.levelno) + self.formatted + self.color_reset
        return logging.Formatter(string).format(record)

def test_input(file: Path, expected_retcode: int, prefix_exec=[], args=[]) -> int:
    """Open a file and compares its output to expected_retcode"""
    with open(file, "r", encoding="utf-8") as f:
        logger.debug(f"Test with {file} ...")
        process = run(
            [*prefix_exec, EXECUTABLE, *args],
            capture_output=True, text=True,
            input=f.read(), check=False
        )

        if process.returncode == expected_retcode:
            logger.info(process.stderr)
        else:
            logger.error(f"Unexpected return code {process.returncode}\n" + (process.stderr))

        return process.returncode

__unittest = True # Silents Python's traceback for unittests
class SyntaxTest(unittest.TestCase):

    def _subtest_files(self, path_glob: str, expected_retcode: int, err_msg: str):
        count = 0
        files = list(Path(".").glob(path_glob))

        for filename in files:
            with self.subTest(str(filename)):
                retcode = test_input(filename, expected_retcode, args=["--only-tree"])
                count += retcode == expected_retcode
                self.assertEqual(retcode, expected_retcode, err_msg)

        log_level = logging.INFO if count == len(files) else logging.WARNING
        logger.log(log_level, f"{count}/{len(files)} of {path_glob} are ok\n")

    def test_valid_syntax_inputs(self):
        logger.debug("# Test valid inputs :")
        self._subtest_files("syn-good/**/*.tpc", 0, "Input was not accepted while it should")

    def test_rejected_syntax_inputs(self):
        logger.debug("# Test rejected inputs :")
        self._subtest_files("syn-err/**/*.tpc", 1, "Input was accepted while it should not")
    
    def _valgrind_conditionnal_jumps(self, path_glob: str, expected_retcode: int):
        """Use valgrind against inputs, to check for conditionnal jumps"""
        files = list(Path(".").glob(path_glob))

        for filename in files:
            with self.subTest(str(filename)):
                retcode = test_input(filename, expected_retcode, ["valgrind", "--error-exitcode=10", "--leak-check=no", "--track-origins=yes"])
                self.assertEqual(retcode, expected_retcode, "Valgrind detected conditionnl jumps")
    
    def test_valid_valgrind_conditionnal_jumps(self):
        logger.debug("# Test valgrind for conditionnal jumps :")
        self._valgrind_conditionnal_jumps("good/random/*.tpc", 0)
        self._valgrind_conditionnal_jumps("syn-err/random/*.tpc", 1)

def parse_args():
    parser = argparse.ArgumentParser(
        prog='Test syntax analyser'
    )
    parser.add_argument(
        "--no-color",
        default=False,
        action="store_true",
        help="Disable colored output"
    )
    # Unknown args are passed to unittest's argparse
    args, unittest_args = parser.parse_known_args()
    sys.argv = [sys.argv[0]] + unittest_args

    return args


if __name__ == '__main__':

    args = parse_args()

    logger.setLevel(logging.DEBUG)
    sh = logging.StreamHandler()
    if not args.no_color:
        sh.setFormatter(ColoredFormatter())
    logger.addHandler(sh)

    unittest.main(verbosity=0)
