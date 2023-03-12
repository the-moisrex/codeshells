import random
from itertools import count
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

index = count()

xvals = []
yvals = []


def animate(i):
    xvals.append(next(index))
    yvals.append(random.randint(0, 5))

    plt.cla()
    plt.plot(xvals, yvals)


ani = FuncAnimation(plt.gcf(),
                    animate,
                    interval=500,
                    cache_frame_data=False)
plt.tight_layout()
plt.show()

