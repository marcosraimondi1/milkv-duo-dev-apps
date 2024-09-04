#!/usr/bin/python
import matplotlib.pyplot as plt
import argparse

# Set up argument parser
parser = argparse.ArgumentParser(description='Plot a histogram from latency data.')
parser.add_argument('filename', type=str, help='The file containing latency data (e.g., data.txt)')
args = parser.parse_args()

# Read the data from the specified file
latencies = []
repetitions = []

print("Reading data from file...")

try:
    with open(args.filename, 'r') as file:
        # Skip the header line
        next(file)
        for line in file:
            latency, repetition = map(int, line.split(','))
            latencies.append(latency)
            repetitions.append(repetition)
except FileNotFoundError:
    print(f"Error: The file '{args.filename}' was not found.")
    exit(1)
except ValueError:
    print(f"Error: The file '{args.filename}' contains invalid data.")
    exit(1)

print("Data read successfully.")

# Plotting the histogram
plt.figure(figsize=(10, 6))
plt.stem(latencies, repetitions)
plt.xlabel('Latency (µs)')
plt.ylabel('Repetitions')
plt.title('Latency Histogram `' + args.filename + '`')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.xticks(range(0, max(latencies) + 1, 100))  # Adjust x-ticks if necessary
plt.tight_layout()

# Display the histogram
plt.show()
