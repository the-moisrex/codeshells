#!/usr/bin/env python3
from matplotlib import pyplot as plt

# Median Developer Salaries by Age
dev_x = [25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35]

dev_y = [38496, 42000, 46752, 49320, 53200,
         56000, 62316, 64928, 67317, 68748, 73752]


"""Module Defenition

main:
    calling the main thing
"""

def main():
    """The Main function"""
    plt.plot(dev_x, dev_y)
    plt.show()


if __name__ == "__main__":
    main()


