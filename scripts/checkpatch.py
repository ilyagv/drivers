#!/usr/bin/env python3

"""
This script checks a patch file for correntness.
"""

import sys
import re
import argparse
from os.path import isfile

class FindData:
    """The class is needed for finding data in a text"""
    def __init__(self, pattern: str, error: str) -> None:
        self.str = pattern
        self.error = error

    def find(self, text: str) -> int:
        """Find the pattern in the text"""
        pos = text.find(self.str)
        if pos == -1:
            print_error(self.error)

        return pos


SUBJECT = FindData(
    "\nSubject:",
    "Subject not found")

SIGN_OFF = FindData(
    "\nSigned-off-by: ",
    "'Signed-off-by:' tag isn't found")

LVC_MAILING_LIST = FindData(
    "\nCc: lvc-project@linuxtesting.org",
    "'Cc: lvc-project@linuxtesting.org' isn't found. You need to send"
    " a copy to lvc-project mailing list.")

FIXES = FindData(
    "\nFixes: ",
    "'Fixes:' tag isn't found")


SUBJ_PATCH = FindData(
    "[PATCH",
    "[PATCH] header isn't found in the subject.")


DOUBLE_SIGN_AT = FindData(
    "@@",
    "To send a patch form an InfoTeCS account you need"
    " to add double 'At sign' ('@@') info the Subject!"
)

FOUND_BY = FindData(
    "Found by InfoTeCS on behalf of Linux Verification Center",
    "\'Found by\' message isn't found. For example:\n"
    "Found by InfoTeCS on behalf of Linux Verification Center\n"
    "(linuxtesting.org) with SVACE."
)


def print_error(message: str) -> None:
    """Prints error message and exit"""
    print(f"Error: {message}")
    sys.exit(1)

def check_double_sign_at(subject: str) -> None:
    """Checks if @@ is present in the subject"""
    DOUBLE_SIGN_AT.find(subject)


def check_sign_off(text: str) -> None:
    """Checks if sing-off tag is present"""
    SIGN_OFF.find(text)


def check_linux_testing(text: str) -> None:
    """Checks if LVC mailing list is present"""
    LVC_MAILING_LIST.find(text)


def check_fixes(text: str) -> None:
    """Checks if fixes tag is present"""
    FIXES.find(text)


def check_found_by(text: str) -> None:
    """ Checks if 'Found by' message is present"""
    FOUND_BY.find(text)


def check_subject(text: str, no_check_branch) -> None:
    """Checks the subject for correctness"""
    subject_begin = SUBJECT.find(text)
    subject_end = text.find("\n\n", subject_begin + len(SUBJECT.str))
    if subject_end == -1:
        print_error("The end of the Subject isn't found")

    subject = text[subject_begin + 1:subject_end]
    check_double_sign_at(subject)
    SUBJ_PATCH.find(subject)

    if no_check_branch is False:
        result = re.search(r"\[PATCH (net|net-next).*\]", subject)
        if not result:
            print_error("Target tree name isn't specified in the subject."
                        " Available tree: net and net-next."
                        " To specify target tree name use the prefix flag:\n"
                        "git format-patch "
                        "--subject-prefix='PATCH net-next' start..finish")


def parse(file_name: str, text: str, no_check_branch) -> None:
    """Checks patch text"""
    check_subject(text, no_check_branch)
    check_sign_off(text)
    check_linux_testing(text)
    check_fixes(text)
    check_found_by(text)
    print(f"Done. The patch file '{file_name}' doesn't contain any errors")


def is_ascii(char)->bool:
    """ Check byte is ascii char """
    code_pos = ord(char)
    return code_pos in range(128)

def print_non_ascii(string: str, line_number: int) -> None:
    """ Print non ascii characters in the string """
    byte_pos = 0
    for char in string:
        byte_pos += 1
        if not is_ascii(char):
            char_hex = hex(ord(char))
            print(f"Error in line {line_number} pos {byte_pos}."
                f" Character \'{char}\'({char_hex}) isn't an ascii character.")
            print(string)
            print((byte_pos - 1) * ' ' + str('^'))

def find_non_ascii(file_name: str)-> None:
    """ Print non acrii characters in the file """
    #with open(file_name, 'rb') as file:
    with open(file_name, 'r', encoding='utf-8') as file:
        lines = file.readlines()
        line_number = 1
        for line in lines:
            print_non_ascii(line.strip("\n"), line_number)
            line_number += 1

def parse_file(file_name: str, no_check_branch) -> None:
    """Loads and parses a patch file"""
    try:
        with open(file_name, 'r', encoding="ascii") as file:
            content = file.read()
            parse(file_name, content, no_check_branch)
    except UnicodeDecodeError:
        find_non_ascii(file_name)


def main() -> None:
    """Main function"""
    parser = argparse.ArgumentParser(description='Check patch script')
    parser.add_argument("patch", type=str, help="Patch file path")
    parser.add_argument("--no-check-branch", \
            action="store_true", help="don't check a branch in the subject")
    options = parser.parse_args()

    if not isfile(options.patch):
        print_error(f"File '{options.patch}' doesn't exist")

    parse_file(options.patch, options.no_check_branch)


if __name__ == "__main__":
    main()
