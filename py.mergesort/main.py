#!/usr/bin/env python3

"""Module Defenition

main:
    calling the main thing
"""
def merge(arr1, arr2):
    res = []
    lrs = []
    print()
    print("arr1: %d %s; arr2: %d %s" % (len(arr1), arr1, len(arr2), arr2))
    while (len(arr1) > 0) and (len(arr2) > 0):
        if arr1[0] < arr2[0]:
            lrs.append("L")
            res.append(arr1[0])
            arr1.pop(0)
        else:
            lrs.append("R")
            res.append(arr2[0])
            arr2.pop(0)
    res += arr1
    res += arr2
    print("arr1: %d %s; arr2: %d %s --- res: %s" % (len(arr1), arr1, len(arr2), arr2, res))
    return res, lrs

def mergesort(arr, lrs):
    n = len(arr)
    if n < 2:
        return arr, lrs

    mid = int(n / 2)
    left = [arr[i] for i in range(mid)]
    right = [arr[i] for i in range(mid, n)]

    left, lrs = mergesort(left, lrs)
    right, lrs = mergesort(right, lrs)

    arr, lrs2 = merge(left, right)
    lrs += lrs2
    return arr, lrs


#########################################################


def demerge(arr, lrs):
    mid = int(len(arr) / 2)
    left = []
    right = []
    related_lrs_beg = len(lrs) - (len(arr) - 1)

    while len(lrs) != related_lrs_beg:
        print("%s \t\t %s %s | %s" % (lrs[related_lrs_beg:], left, right, arr))
        if lrs[related_lrs_beg] == "L":
            left.append(arr[0])
        else:
            right.append(arr[0])
        arr.pop(0)
        lrs.pop(related_lrs_beg)
        print("%d %s    \t\t %s %s | %s" % (related_lrs_beg, lrs[related_lrs_beg:], left, right, arr))
        print()

    print("---- left: %s ---- right: %s" % (left, right))
    # add the remaining item
    if len(arr) > 0:
        if len(right) < mid or len(right) == len(left):
            right.append(arr[0])
        else:
            left.append(arr[0])
        arr.pop(0)

    print("---- left: %s ---- right: %s" % (left, right))
    res = []
    res += left
    res += right
    mid = int(len(res) / 2)
    new_left = res[:mid]
    new_right = res[mid:]
    print("---- left: %s ---- right: %s" % (new_left, new_right))
    return new_left, new_right, lrs

def demergesort(arr, lrs):
    n = len(arr)
    if n < 2:
        return arr, lrs

    left, right, lrs = demerge(arr, lrs)

    right, lrs = demergesort(right, lrs)
    left, lrs = demergesort(left, lrs)

    res = []
    res += left
    res += right

    return res, lrs
########################################################

def main(arr):
    """The Main function"""
    lrs = []
    mergesort(arr, lrs)
    return len(arr), lrs

if __name__ == "__main__":
    arr = [2, 4, 3, 1, 6, 5]
    print("arr: %s" % arr)
    n, l = main(arr)
    print("n = %d" % n)
    print("l = %s" % l)
    res = list(range(1, n + 1))
    res, _ = demergesort(res, l)
    print("res = %s" % res)
