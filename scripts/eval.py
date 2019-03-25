import pandas as pd
import numpy as np
import math
import os.path

'''
TODOS:
- Care about lost packets using STM Matched
'''


def print_stats_for(path):
    # file = pd.read_csv("1024Byte.csv", skipinitialspace=True,
    #                                   usecols=['Time[s]', 'STM RRT', 'STM Matched', 'Aurix matched'])
    if not os.path.isfile(path):
        print('File ' + path + 'does not exist.')
        return

    data = pd.read_csv(path, skipinitialspace=True)
    if len(data.iloc[0]) != 2:
        raise Exception("Only [Times[i], RTT] supported for now.")

    row_counter = 0
    # Skip until first send
    while data.iloc[row_counter][1] == 0:
        row_counter = row_counter + 1

    # Drop first measurement
    row_counter = row_counter + 2

    durations = []
    # Squash two rows to calculate RTT
    for row in range(row_counter, len(data) - 1, 2):
        duration_in_sec = data.iloc[row+1][0] - data.iloc[row][0]
        duration_in_us = duration_in_sec*1000000
        durations.append(round(duration_in_us, 0))

    samples = len(durations)
    std_dev = np.std(durations)
    mean_val = np.mean(durations)
    min_dur = min(durations)
    max_dur = max(durations)
    q_50 = math.ceil(np.quantile(durations, .50))
    q_90 = math.ceil(np.quantile(durations, .90))
    q_99 = math.ceil(np.quantile(durations, .99))
    q_99_9 = math.ceil(np.quantile(durations, .999))
    print('%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f ' %
          (samples, std_dev, mean_val, min_dur, q_50, q_90, q_99, q_99_9, max_dur), flush=True)


def main():
    tested_num_bytes = [16, 32, 64, 128, 256, 512, 1024]
    print('   Bytes, Samples,   stdev,    mean,     min,     50%,     90%,     99%,  99.99%,     max')

    for num_bytes in tested_num_bytes:
        print('%8u,' % num_bytes, end='')
        print_stats_for(str(num_bytes) + 'Bytes.csv')

    print("Done")


if __name__ == "__main__":
    main()
