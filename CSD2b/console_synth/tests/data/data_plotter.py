


import matplotlib.pyplot as plt
import numpy as np
import csv
import sys
import scipy


def plot_frequency_domain(buffer, sample_rate):
    freq_bins = scipy.fft.fft(buffer)
    magnitudes = np.abs(freq_bins) / len(buffer)
    correspoding_freqs = np.zeros(len(buffer))

    for i in range(len(buffer)):
        correspoding_freqs[i] = i * sample_rate / len(buffer)

    plt.xlabel("frequency (hz)")
    plt.ylabel("magnitude")
    plt.plot(correspoding_freqs, magnitudes)
    plt.show()


def plot_time_domain(buffer):
    plt.xlabel("time in samples")
    plt.ylabel("magnitude")
    plt.plot(buffer)
    plt.show()


if __name__ == '__main__':
    sample_rate = int(sys.argv[1])
    in_file = sys.argv[2]

    data = []

    with open(in_file, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            data.append(float(row[0]))

    np_array = np.array(data)

    plot_time_domain(np_array)
    plot_frequency_domain(np_array, sample_rate)
