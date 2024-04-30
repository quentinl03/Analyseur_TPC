#!/bin/python

import unittest
import logging
import os
import sys
import argparse
from pathlib import Path
from subprocess import run
from typing import Tuple, List
from dataclasses import dataclass
from collections import namedtuple
import re
from unittest import skip

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

CompileResults = namedtuple('CompileResults', ['nb_warnings', 'nb_errors'])

@dataclass(eq=True)
class SourceCodeStats:
    result: 'CompileResults' = None
    expected: 'CompileResults' = None

    @staticmethod
    def from_file(src_code: str, compiler_res: str) -> 'SourceCodeStats':
        """Create a SourceCodeStats that will be used to contain results
        and expected output

        Args:
            src_code (str): Source file as a string
            compiler_res (str): Compiler's output

        Returns:
            SourceCodeStats:
        """
        scs = SourceCodeStats()

        def extract_attr(attribute: str, text: str):
            """Extract a comment from a source file

            Args:
                attribute (str): Attribute : "nb_warning", "nb_errors"
                text (str): Source file

            Returns:
                _type_: int
            """
            pattern = fr'//\s*{attribute}=(\d+)'
            match = re.search(pattern, text)
            if match:
                return int(match.group(1))
            else:
                return None
        scs.expected = CompileResults(
            extract_attr("nb_warnings", src_code),
            extract_attr("nb_errors", src_code)
        )
        
        counter = lambda name, text : len(re.findall(fr".*{name}:.*", text))
        scs.result = CompileResults(
            counter("warning", compiler_res),
            counter("error", compiler_res)
        )

        return scs

    def is_present(self) -> bool:
        """Check if a source file had informations about the
        number of warnings and errors

        Returns:
            bool: True if the source file can be checked to compare the
            compiler's output
        """
        return all(e is not None for e in self.expected)

    def is_ok(self) -> bool:
        """Check if the result of the compiler's output is expected

        Returns:
            bool: true if the result is correct
        """
        return self.expected == self.result

def test_input(file: Path, expected_retcode: int, prefix_exec=[], args=[]) -> Tuple[int, SourceCodeStats]:
    """Open a file and compares its output to expected_retcode

    Args:
        file (Path): File to open
        expected_retcode (int): Expected return code from the compiler
        prefix_exec (list, optional): Add something before the command to run the compiler
        It is used to run valgrind. Defaults to [].
        args (list, optional): Arguments to pass to the compiler. Defaults to [].

    Returns:
        Tuple[int, SourceCodeStats]: return code, and the result of the compiler's
        warning/error emission
    """
    with open(file, "r", encoding="utf-8") as f:
        src_code = f.read()
        logger.debug(f"Test with {file} ...")
        process = run(
            [*prefix_exec, EXECUTABLE, *args],
            capture_output=True, text=True,
            input=src_code, check=False
        )

        scs = SourceCodeStats.from_file(src_code, process.stderr)

        if process.returncode == expected_retcode:
            logger.info(process.stderr)
        else:
            logger.error(f"Unexpected return code {process.returncode}\n" + (process.stderr))

        return process.returncode, scs

#__unittest = True # Silents Python's traceback for unittests
class SyntaxTest(unittest.TestCase):

    def _subtest_files(self, args: List[str], path_glob: str, expected_retcode: int, err_msg: str):
        count = 0
        files = sorted(list(Path(".").glob(path_glob)))

        for filename in files:
            with self.subTest(str(filename)):
                retcode, scs = test_input(filename, expected_retcode, args=args)
                count += retcode == expected_retcode
                self.assertEqual(retcode, expected_retcode, err_msg)

                if scs.is_present():
                    self.assertTrue(
                        scs.is_ok(),
                        "Warn/Error comments were found, but expected values are not equal. "
                        f"Expected {scs.expected}, got {scs.result}"
                    )

        log_level = logging.INFO if count == len(files) else logging.WARNING
        logger.log(log_level, f"{count}/{len(files)} of {path_glob} are ok\n")

    def test_0_valid_syntax_inputs(self):
        logger.debug("# Test syntaxically valid inputs :")
        self._subtest_files(["--only-tree"], "syn-good/**/*.tpc", 0, "Syntax was not accepted while it should")

    def test_1_rejected_syntax_inputs(self):
        logger.debug("# Test syntaxically rejected inputs :")
        self._subtest_files(["--only-tree"], "syn-err/**/*.tpc", 1, "Syntax was accepted while it should not")

    def test_2_semantic_errors(self):
        logger.debug("# Test semantic errors :")
        self._subtest_files(["--only-semantic"], "sem-err/**/*.tpc", 2, "At least one semantic error was expected but nothing was detected")

    def test_3_semantic_warnings(self):
        logger.debug("# Test warning emission :")
        self._subtest_files(["--only-semantic"], "warn/**/*.tpc", 0, "A warning caused an error different than 0")

    def test_4_semantic_good(self):
        logger.debug("# Test semantically good source code")
        files = list(Path(".").glob("good/**/*.tpc"))

        for filename in files:
            with self.subTest(str(filename)):
                retcode, scs = test_input(filename, 0, args=["--only-semantic"])
                self.assertEqual(retcode, 0, "")
                self.assertEqual(scs.result, CompileResults(0, 0))

    def _valgrind_conditionnal_jumps(self, path_glob: str, expected_retcode: int):
        """Use valgrind against inputs, to check for conditionnal jumps"""
        files = list(Path(".").glob(path_glob))

        for filename in files:
            with self.subTest(str(filename)):
                retcode, _ = test_input(filename, expected_retcode, ["valgrind", "--error-exitcode=10", "--leak-check=no", "--track-origins=yes"])
                self.assertEqual(retcode, expected_retcode, "Valgrind detected conditionnl jumps")

    # @skip
    def test_5_valid_valgrind_conditionnal_jumps(self):
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
