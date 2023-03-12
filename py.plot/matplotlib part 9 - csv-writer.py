import csv
import time
import random

xvalue = 0
total_1 = 1000
total_2 = 1000

filepath = "/tmp/csv-file.csv"

fieldnames = ["xvalues", "total_1", "total_2"]

with open(filepath, 'w') as csv_file:
    csv_writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    csv_writer.writeheader()

while True:
    with open(filepath, 'a') as csv_file:
        csv_writer = csv.DictWriter(csv_file, fieldnames=fieldnames)

        info = {
            "xvalues": xvalue,
            "total_1": total_1,
            "total_2": total_2
        }

        csv_writer.writerow(info)
        print(xvalue, total_1, total_2)

        xvalue += 1
        total_1 += random.randint(-6, 8)
        total_2 += random.randint(-5, 6)

    time.sleep(1)
