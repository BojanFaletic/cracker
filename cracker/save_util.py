from cracker.constants import PLOT_FILE, NUMPY_FILE
import numpy as np
import matplotlib.pyplot as plt


def save_average_plot(time_: np.array, min_: np.array, max_: np.array,
                      average_: np.array, label_: str) -> None:
    plot_name_avg = PLOT_FILE + label_ + '_min_max_avg.png'
    plt.plot(time_, min_, time_, max_, time_, average_)
    plt.savefig(plot_name_avg)
    plt.close()


def save_std_pot(time_: np.array, std_: np.array, label_: str) -> None:
    plot_name_std = PLOT_FILE + label_ + '_avg_std.png'
    plt.plot(time_, std_)
    plt.savefig(plot_name_std)
    plt.close()


def save_plot_data(data_: np.array, label_: str) -> None:
    min_v = data_.T[0]
    max_v = data_.T[1]
    average_v = data_.T[2]
    std_v = data_.T[3]
    t = np.arange(256)

    save_average_plot(t, min_v, max_v, average_v, label_)
    save_std_pot(t, std_v, label_)


def save_raw_data(data_: np.array, label_: str) -> None:
    check_name = NUMPY_FILE + label_ + '.npy'
    np.save(check_name, data_)


def save_data(data_: np.array, label_: str) -> None:
    save_plot_data(data_, label_)
    save_raw_data(data_, label_)
