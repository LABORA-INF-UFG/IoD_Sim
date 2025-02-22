import psutil
import time
import csv
import subprocess
from datetime import datetime

stop_flag = False

def collect_system_usage(pid):
    global stop_flag
    # Configuration
    INTERVAL = 0  # Time interval in seconds between each data collection
    TIMESTAMP = datetime.now().strftime("%Y%m%d_%H%M%S")
    OUTPUT_FILE = f"system_usage_data_{TIMESTAMP}.csv"

    # Open the CSV file for writing
    with open(OUTPUT_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write the header row
        writer.writerow(["Elapsed Time (s)", "System CPU Usage (%)", "Process RAM Usage (MB)", "Process CPU Usage (%)"])

        start_time = time.time()  # Record the start time

        print("Collecting system usage data...")

        process = psutil.Process(pid)  # Get the process object

        while not stop_flag:
            # Calculate elapsed time in seconds
            elapsed_time = time.time() - start_time

            # Get CPU usage as a percentage
            cpu_usage = psutil.cpu_percent(interval=1.0)
            process_ram_usage = process.memory_info().rss / (1024 * 1024)  # Process RAM usage in MB

            # Get process CPU usage using the command
            process_cpu_usage = subprocess.check_output("top -n1 | grep ns3.38-iodsim-d | awk '{print $9}'", shell=True).decode().strip()
            if process_cpu_usage == "S":
                process_cpu_usage = subprocess.check_output("top -n1 | grep ns3.38-iodsim-d | awk '{print $10}'", shell=True).decode().strip()
            # Write the data to the CSV file
            writer.writerow([round(elapsed_time, 2), cpu_usage, round(process_ram_usage, 2), process_cpu_usage])

            # Print the data to the console (optional)
            print(f"Elapsed Time: {round(elapsed_time, 2)}s, System CPU Usage: {cpu_usage}%, Process RAM Usage: {round(process_ram_usage, 2)} MB, Process CPU Usage: {process_cpu_usage}%")

            # Wait for the next interval
            #time.sleep(INTERVAL)

    print(f"Data collection complete. Data saved to {OUTPUT_FILE}")

def stop_collecting_system_usage():
    global stop_flag
    stop_flag = True

if __name__ == "__main__":
    pid = int(input("Enter the process PID: "))
    collect_system_usage(pid)