import random
from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

filepath = "/tmp/csv-file.csv"
index = count()


def animate(i):
    data = pd.read_csv(filepath)
    x = data['xvalues']
    y1 = data['total_1']
    y2 = data['total_2']

    plt.cla()
    plt.plot(x, y1, label="Channel 1")
    plt.plot(x, y2, label="Channel 2")

    plt.legend(loc="upper left")
    plt.tight_layout()


ani = FuncAnimation(plt.gcf(),
                    animate,
                    interval=500,
                    cache_frame_data=False)
plt.show()
