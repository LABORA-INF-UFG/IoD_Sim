import pandas as pd
import glob
import matplotlib.pyplot as plt

def calculate_global_mean_and_elapsed_time(base_name):
    # Find all files matching the base name pattern
    files = glob.glob(f"{base_name}*.csv")
    all_data = pd.DataFrame()
    max_elapsed_times = []  # List to store the max elapsed time from each file

    # Read and concatenate all files into one DataFrame
    for file in files:
        df = pd.read_csv(file)
        all_data = pd.concat([all_data, df], ignore_index=True)
        # Append the max elapsed time from the current file
        max_elapsed_times.append(df["Elapsed Time (s)"].max())

    # Calculate the global mean of each column
    global_means = all_data.mean()
    # Calculate the mean of the max elapsed times
    mean_max_elapsed_time = pd.Series(max_elapsed_times).mean()

    return global_means, mean_max_elapsed_time

# List of filename patterns and corresponding drone amounts
filename_patterns = [
    'system_usage_exp3_01',
    'system_usage_exp3_5',
    'system_usage_exp3_10',
    'system_usage_exp3_15',
    'system_usage_exp3_20',
    'system_usage_exp3_25'
]
drone_amounts = [1, 5, 10, 15, 20, 25]

# Store results for each filename pattern
results = {}
for pattern in filename_patterns:
    global_mean, mean_elapsed_time = calculate_global_mean_and_elapsed_time(pattern)
    results[pattern] = {
        'global_mean': global_mean,
        'mean_elapsed_time': mean_elapsed_time
    }

# Extract data for plotting
ram_usage = [results[pattern]['global_mean']['Process RAM Usage (MB)'] for pattern in filename_patterns]
elapsed_time = [results[pattern]['mean_elapsed_time'] + 1 for pattern in filename_patterns]
system_cpu_usage = [results[pattern]['global_mean']['System CPU Usage (%)'] for pattern in filename_patterns]
process_cpu_usage = [results[pattern]['global_mean']['Process CPU Usage (%)'] for pattern in filename_patterns]

# Plotting
plt.rcParams.update({'font.size': 12})
plt.figure(figsize=(18, 5))

# Graph 1: Process RAM Usage (MB)
plt.subplot(1, 3, 2)
plt.plot(drone_amounts, ram_usage, marker='o', linestyle='-', color='tab:blue', label='E-IoD-Sim')
plt.xlabel('Number of Drones')
plt.ylabel('RAM Usage (MB)')
#plt.title('E-IoD-Sim RAM Usage vs Number of Drones')
plt.grid(True)
plt.legend()

# Graph 2: Elapsed Time (s)
plt.subplot(1, 3, 1)
plt.plot(drone_amounts, elapsed_time, marker='o', linestyle='-', color='tab:blue', label='Real Time')
plt.plot(drone_amounts, [60, 60, 60, 60, 60, 60], linestyle='--', color='tab:orange', label='Real Time')
plt.xlabel('Number of Drones')
plt.ylabel('Elapsed Time (s)')
#plt.title('Elapsed Time vs Number of Drones')
plt.grid(True)
plt.legend()

# Graph 3: System CPU Usage and Process CPU Usage
plt.subplot(1, 3, 3)
plt.plot(drone_amounts, system_cpu_usage, marker='o', linestyle='-', color='tab:blue', label='System')
plt.plot(drone_amounts, process_cpu_usage, marker='o', linestyle='-', color='tab:orange', label='E-IoD-Sim')
plt.xlabel('Number of Drones')
plt.ylabel('CPU Usage (%)')
#plt.title('System and Process CPU Usage vs Number of Drones')
plt.grid(True)
plt.legend()

# Display the plots
plt.tight_layout()
plt.show()
plt.savefig('results.pdf')