import requests
import json
import time
import os
from threading import Thread
from collections import defaultdict
import subprocess
from collect_system_usage import collect_system_usage, stop_collecting_system_usage  # Import the functions

working_directory = os.getcwd()
config_values = {}
config_values['iod_addr'] = "http://127.0.0.1:18080/"
config_values['scenario_json'] = f"{working_directory}/scenario/lm_demo_gui.json"
config_values['change_route_json'] = f"{working_directory}/scenario/lm_elton_demo_change_route.json"
config_values['flight_plan_json'] = f"{working_directory}/scenario/lm_elton_flight_plan.json"
location_data = defaultdict(list)
zsp_location_data = defaultdict(list)
delay_data = defaultdict(list)
throughput_data = defaultdict(list)
jitter_data = defaultdict(list)
tx_power_data = defaultdict(list)
battery_data = defaultdict(list)
packet_loss_data = defaultdict(list)
sum_delay = {}
sub_delay = {}
sum_sendp = {}
sum_lostp = {}
iodsim_pid = None

def start_scenario_visualizer():
    if working_directory.endswith("IoD_Sim"):
        return subprocess.Popen(f'python3 {working_directory}/iod_gui/scenario_visualizer.py', shell=True)
    if working_directory.endswith("ns3"):
        return subprocess.Popen(f'python3 ../iod_gui/scenario_visualizer.py', shell=True)
    if working_directory.endswith("iod_gui"):
        return subprocess.Popen(f'python3 scenario_visualizer.py', shell=True)


def start_iodsim():
    global iodsim_pid
    if working_directory.endswith("IoD_Sim"):
        process = subprocess.Popen(f'cd ns3/ && ./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)
        iodsim_pid = process.pid
        return process
        return subprocess.Popen(f'cd ns3/ && ./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)
    if working_directory.endswith("ns3"):
        process = subprocess.Popen(f'./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)
        iodsim_pid = process.pid
        return process
        return subprocess.Popen(f'./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)
    if working_directory.endswith("iod_gui"):
        process = subprocess.Popen(f'cd ../ns3 && ./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)
        iodsim_pid = process.pid
        return process
        return subprocess.Popen(f'cd ../ns3 && ./ns3 run iodsim', shell=True, stdout=subprocess.PIPE)


def call_rest_api(method, address, content=None):
    try:
        response = None

        if method == 'GET':
            response = requests.get(address)
        elif method == 'POST':
            response = requests.post(address, json=json.loads(content))
        elif method == 'DELETE':
            response = requests.delete(address)
        else:
            raise ValueError(f"Invalid HTTP method: {method}")

        return response

    except requests.exceptions.RequestException as e:
        print(f"Error occurred while making the request: {e}")
        return None


def format_delay(delay):
    float_value = float(delay[:-2])
    return round(float_value / 10000000, 2)  # get decimals correctely


def format_jitter(jitter):
    dot_index = jitter.find("e") - 1
    return round(float(jitter[1:dot_index]), 2)


def request_metrics():
    global location_data, zsp_location_data, delay_data, throughput_data, jitter_data, tx_power_data, battery_data, packet_loss_data, sum_delay, sum_sendp, sum_lostp
    method = 'GET'
    address = 'http://127.0.0.1:18080/get_realtime_metrics'
    result = call_rest_api(method, address)
    # Start 'collect_system_usage' function in a separate thread
    usage_thread = Thread(target=collect_system_usage, args=(iodsim_pid,))
    usage_thread.start()
    while result != None:
        if result.status_code != 200:
            print(f"Result: \n[{result.status_code}] {result.text}")
            break
        time.sleep(0.5)
        new_data = json.loads(result.text)

        for data in new_data['zsps-metrics']:
            id_value = data.get('id')
            time_value = data.get('time')
            zsp_location_value = data.get('location')
            zsp_location_data[id_value].append(
                    (time_value, zsp_location_value))



        for data in new_data['drones-metrics']:
            id_value = data.get('node-id')
            time_value = data.get('time')
            network = data.get('network', {})
            location_value = data.get('location')
            if id_value is not None and time_value is not None and network:
                if not id_value in sum_delay:
                    sum_delay[id_value] = 0
                    sub_delay[id_value] = 0
                    sum_sendp[id_value] = 0
                    sum_lostp[id_value] = 0

                # print(f"old delay: {r_delay} \t| \tnew delay: {delay} \t| \tresult: {round(float(format_delay(network.get('delay')) - delay))}")
                delay_value = round(float(format_delay(network.get('delay'))))
                if (delay_value > sum_delay[id_value]):
                    sub_delay[id_value] = delay_value - sum_delay[id_value]
                    sum_delay[id_value] = delay_value
                throughput_value = round(
                    float(network.get('throughput-kbps')), 2)
                sentp = round(float(network.get('sent-pck')), 2)
                lostp = round(float(network.get('lost-pck')), 2)
                if (sentp > sum_sendp[id_value]):
                    sub = sentp - sum_sendp[id_value]
                    sum_sendp[id_value] = sentp
                    sentp = sub
                if (lostp > sum_lostp[id_value]):
                    sub = lostp - sum_lostp[id_value]
                    sum_lostp[id_value] = lostp
                    lostp = sub
                if (lostp == sum_lostp[id_value]):
                    lostp = 0


                delay_data[id_value].append((time_value, sub_delay[id_value]))
                throughput_data[id_value].append(
                    (time_value, throughput_value))
                packet_loss_data[id_value].append(
                    (time_value, round(lostp/(sentp), 2)*100))
                jitter_data[id_value].append(
                    (time_value, format_jitter(network.get('jitter'))))
                tx_power_data[id_value].append(
                    (time_value, round(network.get('txPower'), 2)))
                battery_data[id_value].append(
                    (time_value, data.get('battery')))
                location_data[id_value].append(
                    (time_value, location_value))

        result = call_rest_api(method, address)
    # Stop 'collect_system_usage' function
    stop_collecting_system_usage()
    usage_thread.join()


request_metrics_thread = Thread(target=request_metrics)

if __name__ == '__main__':
    # print(call_rest_api('GET', 'http://127.0.0.1:18080/get_realtime_metrics').text)
    # request_metrics_thread.start()
    # request_metrics_thread.join()
    start_scenario_visualizer()
