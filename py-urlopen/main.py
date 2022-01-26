#!/usr/bin/env python3

"""Module Defenition

main:
    calling the main thing
"""

from urllib.request import urlopen;

def main(url):
    """The Main function"""
    with urlopen(url) as data:
        for line in data:
            print(line.decode('utf-8'))


if __name__ == "__main__":
    main("https://sixty-north.com/c/t.txt")


