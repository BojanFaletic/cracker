from cracker.constants import PLOT_DIR, RAW_DIR
import numpy as np
import matplotlib.pyplot as plt


def save_plot_data(data_: np.array, label_: str) -> None:
    plot_name_avg = PLOT_DIR + label_ + '_min_max_avg.png'
    plot_name_std = PLOT_DIR + label_ + '_avg_std.png'

    min_v = data_.T[0]
    max_v = data_.T[1]
    average_v = data_.T[2]
    std_v = data_.T[3]
    t = np.arange(256)

    plt.plot(t, min_v, t, max_v, t, average_v)
    plt.savefig(plot_name_avg)

    plt.plot(t, average_v, t, std_v)
    plt.savefig(plot_name_std)


def save_raw_data(data_: np.array, label_: str) -> None:
    check_name = RAW_DIR + label_ + '.npy'
    np.save(check_name, data_)


def save_data(data_: np.array, label_: str) -> None:
    save_plot_data(data_, label_)
    save_raw_data(data_, label_)
