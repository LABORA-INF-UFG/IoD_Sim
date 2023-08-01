import time
import PySimpleGUI as sg
from controller import call_rest_api, delay_data, throughput_data, jitter_data, tx_power_data, battery_data, packet_loss_data, request_metrics_thread, config_values, working_directory
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from pathlib import Path
from threading import Thread
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TkAgg')

sg.theme('LightGrey1')
# common
update_options = {'Change Route': 'change_position'}
figures_window_alive = False
update_figures_event_thread = None
fig = None
axes = None
fig_agg = None


def draw_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg


def create_update_figure_event(window, sleep_time=0.3):
    while request_metrics_thread.is_alive():
        window.write_event_value('Update figure', None)
        time.sleep(sleep_time)


def update_figure():
    global fig, axes, fig_agg
    axes[0, 0].cla()
    axes[0, 1].cla()
    axes[1, 0].cla()
    axes[1, 1].cla()
    axes[2, 0].cla()
    axes[2, 1].cla()

    for id_value, delay_points in delay_data.items():
        time_values, delay_values = zip(*delay_points)
        axes[0, 0].plot(time_values, delay_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    for id_value, throughput_points in throughput_data.items():
        time_values, throughput_values = zip(*throughput_points)
        axes[0, 1].plot(time_values, throughput_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    for id_value, jitter_points in jitter_data.items():
        time_values, jitter_values = zip(*jitter_points)
        axes[1, 0].plot(time_values, jitter_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    for id_value, tx_power_points in tx_power_data.items():
        time_values, tx_power_values = zip(*tx_power_points)
        axes[1, 1].plot(time_values, tx_power_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    for id_value, battery_points in battery_data.items():
        time_values, battery_values = zip(*battery_points)
        axes[2, 0].plot(time_values, battery_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    for id_value, packet_loss_points in packet_loss_data.items():
        time_values, packet_loss_values = zip(*packet_loss_points)
        axes[2, 1].plot(time_values, packet_loss_values, 'o-',
                        linestyle='dashed', label=f'ID {id_value}')

    # axes[0, 0].set_title('Delay Over Time')
    axes[0, 0].legend()
    axes[0, 0].grid(True)
    # axes[0, 0].set_xlabel('Time (s)')
    axes[0, 0].set_ylabel('Delay (ns)')

    # axes[0, 1].set_title('Throughput Over Time')
    axes[0, 1].legend()
    axes[0, 1].grid(True)
    # axes[0, 1].set_xlabel('Time (s)')
    axes[0, 1].set_ylabel('Throughput (kbps)')

    # axes[1, 0].set_title('Jitter Over Time')
    axes[1, 0].legend()
    axes[1, 0].grid(True)
    # axes[1, 0].set_xlabel('Time (s)')
    axes[1, 0].set_ylabel('Jitter (ns)')

    # axes[1, 1].set_title('TX Power Time')
    axes[1, 1].legend()
    axes[1, 1].grid(True)
    # axes[1, 1].set_xlabel('Time (s)')
    axes[1, 1].set_ylabel('TX Power (dBm)')

    # axes[2, 0].set_title('Battery Over Time')
    axes[2, 0].legend()
    axes[2, 0].grid(True)
    axes[2, 0].set_xlabel('Time (s)')
    axes[2, 0].set_ylabel('Battery (mAh)')

    # axes[2, 1].set_title('Packet loss Over Time')
    axes[2, 1].legend()
    axes[2, 1].grid(True)
    axes[2, 1].set_xlabel('Time (s)')
    axes[2, 1].set_ylabel('Packet loss (%)')

    fig_agg.draw()


def create_window1():
    # simulation controller
    layout_control = [
        [sg.Text("Scenario configuration:")],
        [sg.InputText(key='-SCENARIO_FILE_PATH-', default_text=config_values['scenario_json']),
         sg.FileBrowse(button_text="Browse", initial_folder=working_directory,
                       file_types=[("JSON Files", "*.json"), ("All the files", "*.*")]),
         sg.Button("Import")],
        [sg.Multiline(size=(80, 10), key='-SCENARIO_CONFIGURATION-')],
        [sg.Button("Start"), sg.Button("Stop")],
    ]

    # simulation updater
    combo_list = list(update_options.keys())
    layout_updater = [
        [sg.Text("Select operation:"), sg.InputCombo(
            combo_list, size=(20, 1), key='-UPDATE_OPTION-', default_value=combo_list[0])],
        [sg.Text("Request content:")],
        [sg.Multiline(size=(80, 10), key='-UPDATE_CONTENT-')],
        [sg.Button("Request Update")]
    ]

    # config
    layout_config = [
        [sg.Text("IoD_Sim address:"), sg.InputText(
            size=(30, 1), key='-IOD_ADDRESS-', default_text=config_values['iod_addr'])],
        [sg.Text("Scenario .json file:"), sg.InputText(
            size=(30, 1), key='-SCENARIO_JSON-', default_text=config_values['scenario_json'])],
        [sg.Text("Change route .json file:"), sg.InputText(
            size=(30, 1), key='-CHANGE_ROUTE_JSON-', default_text=config_values['change_route_json'])],
        [sg.Button("Update")]
    ]

    # tab
    tab_group = [
        [sg.TabGroup(
            [
                [sg.Tab('Initiate experiment', layout_control,
                        tooltip='Start and stop simulation')],
                [sg.Tab('Update flight plan', layout_updater,
                        tooltip='Change realtime simulation elements')],
                [sg.Tab('Configuration', layout_config,
                        tooltip='Configure IoD_Sim values')]
            ])
         ]
    ]
    return sg.Window("IoD_Sim Dashboard",  tab_group, finalize=True)


def create_window2(master):
    global figures_window_alive, fig, axes, fig_agg, update_figures_event_thread
    layout = [[sg.Canvas(size=(640, 480), key='-CANVAS-')]]

  # create the form and show it without the plot
    window = sg.Window('Scenario Visualizer',
                       layout, finalize=True)
    figures_window_alive = True

    canvas_elem = window['-CANVAS-']
    canvas = canvas_elem.TKCanvas

    # draw the initial plot in the window
    fig, axes = plt.subplots(3, 2, figsize=(12, 8), sharex='col')
    # axes[0, 0].set_xlabel('Time (s)')
    axes[0, 0].set_ylabel('Delay (ns)')

    # axes[0, 1].set_xlabel('Time (s)')
    axes[0, 1].set_ylabel('Throughput (kbps)')

    # axes[1, 0].set_xlabel('Time (s)')
    axes[1, 0].set_ylabel('Jitter (ns)')

    # axes[1, 1].set_xlabel('Time (s)')
    axes[1, 1].set_ylabel('TX Power (dBm)')

    axes[2, 0].set_xlabel('Time (s)')
    axes[2, 0].set_ylabel('Battery (mAh)')

    axes[2, 1].set_xlabel('Time (s)')
    axes[2, 1].set_ylabel('Packet loss (%)')

    fig_agg = draw_figure(canvas, fig)

    # logic
    request_metrics_thread.start()
    update_figures_event_thread = Thread(
        target=create_update_figure_event, args=(master, ))
    update_figures_event_thread.start()
    return window


def main_window():
    window1, window2 = create_window1(), None

    while True:
        window, event, values = sg.read_all_windows()

        if event == sg.WIN_CLOSED or event == 'Exit':
            window.close()
            if window == window2:       # if closing win 2, mark as closed
                window2 = None
                if update_figures_event_thread.is_alive():
                    update_figures_event_thread.stop()
                if request_metrics_thread.is_alive():
                    request_metrics_thread.stop()
            elif window == window1:     # if closing win 1, exit program
                if update_figures_event_thread:
                    if update_figures_event_thread.is_alive():
                        update_figures_event_thread.stop()
                    if request_metrics_thread.is_alive():
                        request_metrics_thread.stop()
                break
        elif event == "Start" and not window2:
            if figures_window_alive:
                sg.popup(
                    f"Simulation already running. Stop it first or change to update page.", title='Error')
            else:
                result = call_rest_api(
                    'POST', f"{config_values['iod_addr']}{'simulation'}", content=values['-SCENARIO_CONFIGURATION-'])
                if result == None:
                    sg.popup(
                        f"REST Server not found. Is IoD_Sim running?", title='Error')
                elif result.status_code == 200:
                    window2 = create_window2(window1)
                else:
                    sg.popup(
                        f"Unspected error: [{result.status_code}] {result.text}", title='Error')
        elif event == "Stop":
            if figures_window_alive:
                result = call_rest_api(
                    'DELETE', f"{config_values['iod_addr']}{'simulation'}")
                if result == None:
                    sg.popup(
                        f"REST Server not found. Is IoD_Sim running?", title='Error')
                elif result.status_code == 200:
                    sg.popup(f"Simulation stopped.", title='Info')
                    if request_metrics_thread.is_alive():
                        request_metrics_thread.stop()
                else:
                    sg.popup(
                        f"Unspected error: [{result.status_code}] {result.text}")
            else:
                sg.popup(
                    f"Simulation is not running. Start it first.", title='Error')
        elif event == "Update":
            config_values['iod_addr'] = values['-IOD_ADDRESS-']
            config_values['scenario_json'] = values['-SCENARIO_JSON-']
            config_values['change_route_json'] = values['-CHANGE_ROUTE_JSON-']
        elif event == "Request Update":
            if figures_window_alive:
                result = call_rest_api(
                    'POST', f"{config_values['iod_addr']}{update_options[values['-UPDATE_OPTION-']]}", content=values['-UPDATE_CONTENT-'])
                if result == None:
                    sg.popup(
                        f"REST Server not found. Is IoD_Sim running?", title='Error')
                elif result.status_code == 200:
                    sg.popup(
                        f"Sucessfully updated.", title='Info')
                else:
                    sg.popup(
                        f"Unspected error: [{result.status_code}] {result.text}", title='Error')
        elif event == "Import":
            filename = values['-SCENARIO_FILE_PATH-']
            if Path(filename).is_file():
                try:
                    with open(filename, "rt", encoding='utf-8') as f:
                        text = f.read()
                    window['-SCENARIO_CONFIGURATION-'].Update(text)
                except Exception as e:
                    sg.popup(f"Error: {e}", title="Error")
        elif event == "Update figure":
            update_figure()
    window.close()


if __name__ == '__main__':
    main_window()
