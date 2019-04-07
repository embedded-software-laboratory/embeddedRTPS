import pandas as pd
import numpy as np
import math
import os.path
import sys
import re

'''
TODOS:
- Care about lost packets using STM Matched
'''
file_ending = 'Bytes.csv'
file_matcher = re.compile(r"^\d+Bytes.csv$") # Names are like 1024Byte.csv

MAX_NUM_SAMPLES = 10000


def print_stats_for(path, num_bytes):
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
        if len(durations) >= MAX_NUM_SAMPLES:
            break
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
    result = '%8u,%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f ' % (
          num_bytes, samples, std_dev, mean_val, min_dur, q_50, q_90, q_99, q_99_9, max_dur)
    print(result, flush=True)
    return result

def evaluate_file(path, filename, file_handle):
	if not file_matcher.match(filename):
		return
	num_bytes = int(filename.split(file_ending, 1)[0])
	result = print_stats_for(os.path.join(path, filename), num_bytes)
	file_handle.write(result + '\n')
    
def get_natural_keys(filename):
	splitted = filename.split(file_ending, 1)
	return int(splitted[0]), file_ending

def main():
	args = sys.argv
	if len(args) < 2:
		print("Need at least one path.\n")
		return

	cwd = os.getcwd()

	for path in args[1:]:
		print("Processing: " + path)
		header = '   Bytes, Samples,   stdev,    mean,     min,     50%,     90%,     99%,  99.99%,     max'
		print(header)

		if path.startswith('./'):
			path = path.split('./', 1)[1]
			path = os.path.join(cwd, path)

		if os.path.isdir(path):
			output_path = os.path.join(path, 'results')
			files = os.listdir(path)
		elif os.path.isfile(path):
			path, filename = os.path.split(path)
			output_path = os.path.join(path, 'results')
			files = [filename]

		file_handle = open(output_path, 'w+')
		file_handle.write(header + '\n')
        
		files = [f for f in files if file_matcher.match(f)]
        
        # Sort with increasing size
		files.sort(key=get_natural_keys)

		for file in files:
			evaluate_file(path, file, file_handle)

		file_handle.close()
        
	print("Done")



if __name__ == "__main__":
    main()
