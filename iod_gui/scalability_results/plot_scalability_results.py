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
fig, axes = plt.subplots(nrows=1, ncols=3, sharey=True, figsize=(10, 3), squeeze=False)

# Graph 1: Process RAM Usage (MB)
axes[0, 1].plot(ram_usage, drone_amounts, marker='o', linestyle='-', color='tab:blue', label='E-IoD-Sim Process')
axes[0, 1].set_xlabel('RAM Usage (MB)')
axes[0, 1].grid(True)
axes[0, 1].legend()

# Graph 2: Elapsed Time (s)
axes[0, 0].plot(elapsed_time, drone_amounts, marker='o', linestyle='-', color='tab:blue', label='Real Time')
axes[0, 0].plot([60, 60, 60, 60, 60, 60], drone_amounts, linestyle='--', color='tab:orange', label='Simulated Time')
axes[0, 0].set_ylabel('Number of Drones')
axes[0, 0].set_xlabel('Elapsed Time (s)')
axes[0, 0].grid(True)
axes[0, 0].legend()

# Graph 3: System CPU Usage and Process CPU Usage
axes[0, 2].plot(system_cpu_usage, drone_amounts, marker='o', linestyle='-', color='tab:blue', label='System')
axes[0, 2].plot(process_cpu_usage, drone_amounts, marker='o', linestyle='-', color='tab:orange', label='E-IoD-Sim Process')
axes[0, 2].set_xlabel('CPU Usage (%)')
axes[0, 2].grid(True)
axes[0, 2].legend()

# Display the plots
plt.tight_layout()
#plt.show()
plt.savefig('exp3_results.pdf', dpi=600, bbox_inches='tight')