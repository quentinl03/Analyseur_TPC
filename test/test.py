#!/bin/python

import unittest
import logging
from pathlib import Path
from subprocess import run

EXECUTABLE = "./bin/tpcas"

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

def test_input(file: Path, expected_retcode: int) -> int:
    """Open a file and compares its output to expected_retcode"""
    with open(file, "r") as f:
        logger.debug(f"Test with {file}...")
        process = run(
            [EXECUTABLE],
            capture_output=True, text=True,
            input=f.read()
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
        files = list(Path("test/").glob(path_glob))

        for filename in files:
            with self.subTest(filename=str(filename)) as sub:
                retcode = test_input(filename, expected_retcode)
                count += retcode == expected_retcode
                self.assertEqual(retcode, expected_retcode, err_msg)

        log_level = logging.INFO if count == len(files) else logging.WARNING
        logger.log(log_level, f"{count}/{len(files)} of {path_glob} are ok\n")

    def test_valid_inputs(self):
        logger.debug("# Test valid inputs :")
        self._subtest_files("good/*", 0, "Input was not accepted while it should")

    def test_rejected_inputs(self):
        logger.debug("# Test rejected inputs :")
        self._subtest_files("syn-err/*", 1, "Input was accepted while it should not")

if __name__ == '__main__':

    logger.setLevel(logging.DEBUG)
    sh = logging.StreamHandler()
    sh.setFormatter(ColoredFormatter())
    logger.addHandler(sh)

    unittest.main(verbosity=0)
