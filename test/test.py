#!/bin/python

import unittest
from pathlib import Path
from subprocess import run

EXECUTABLE = "./bin/tpcas"


def test_input(file: Path) -> int:
    with open(file, "r") as f:
        print(f"Test with {file}")
        process = run(
            [EXECUTABLE],
            capture_output=True, text=True,
            input=f.read()
        )
        # process.stdin.close()
        # affciher stderr
        return process.returncode

class SyntaxTest(unittest.TestCase):

    def _subtest_files(self, path_glob: str, expected_retcode: int, err_msg: str):
        count = 0
        files = list(Path("test/").glob(path_glob))

        for filename in files:
            with self.subTest(filename=str(filename)) as sub:
                retcode = test_input(filename)
                count += retcode == expected_retcode
                self.assertEqual(retcode, expected_retcode, err_msg)

        print(f"{count}/{len(files)} of {path_glob} are ok\n")

    def test_valid_inputs(self):
        self._subtest_files("good/*", 0, "Input was not accepted while it should")

    def test_rejected_inputs(self):
        self._subtest_files("syn-err/*", 1, "Input was accepted while it should not")


if __name__ == '__main__':
    unittest.main(verbosity=0)
