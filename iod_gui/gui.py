import time
import PySimpleGUI as sg
from controller import call_rest_api, request_metrics_thread, config_values, working_directory, start_scenario_visualizer, start_iodsim
from pathlib import Path
sg.theme('LightGrey1')

# common
update_options = {'Change Route': 'change_position'}
# figures_window_alive = False

def create_window():
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
        [sg.InputText(key='-FLIGHT_PLAN_FILE_PATH-', default_text=config_values['flight_plan_json']),
         sg.FileBrowse(button_text="Browse", initial_folder=working_directory,
                       file_types=[("JSON Files", "*.json"), ("All the files", "*.*")]),
         sg.Button("Import", key='-IMPORT_FLIGHT_PLAN-')],
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

def check_rest_result(result):
    if not result: return None


def main_window():
    window = create_window()
    iodsim_running = False
    iodsim_process = None
    while True:
        event, values = window.read()

        if event == sg.WIN_CLOSED or event == 'Exit':
            window.close()
            break
        elif event == "Start":
            if iodsim_running:
                sg.popup(
                    f"Simulation already running. Stop it first or move to update page.", title='Error')
            else:
                result = call_rest_api(
                        'POST', f"{config_values['iod_addr']}{'simulation'}", content=values['-SCENARIO_CONFIGURATION-'])
                if result == None:
                    iodsim_process = start_iodsim()
                    tries = 0
                    while tries < 2 and not result:
                        time.sleep(1)
                        result = call_rest_api(
                            'POST', f"{config_values['iod_addr']}{'simulation'}", content=values['-SCENARIO_CONFIGURATION-'])
                        tries+=1
                    if(not result):
                        sg.popup(
                            f"Failed to start IoD_Sim. Try to start it manually with './ns3 run iodsim'.", title='Error')
                if result:
                    if result.status_code == 200:
                        start_scenario_visualizer()
                        iodsim_running = True
                    else:
                        sg.popup(
                            f"Unspected error: [{result.status_code}] {result.text}", title='Error')
        elif event == "Stop":
            if iodsim_running:
                result = call_rest_api(
                    'DELETE', f"{config_values['iod_addr']}{'simulation'}")
                if result == None:
                    sg.popup(
                        f"Simulation stopped.", title='Error')
                elif result.status_code == 200:
                    result = call_rest_api(
                        'DELETE', f"{config_values['iod_addr']}{'kill'}")
                    sg.popup(f"Simulation stopped.", title='Info')
                else:
                    sg.popup(
                        f"Unspected error: [{result.status_code}] {result.text}")
                iodsim_running = False
            else:
                sg.popup(
                    f"Simulation is not running. Start it first.", title='Error')
        elif event == "Update":
            config_values['iod_addr'] = values['-IOD_ADDRESS-']
            config_values['scenario_json'] = values['-SCENARIO_JSON-']
            config_values['change_route_json'] = values['-CHANGE_ROUTE_JSON-']
        elif event == "Request Update":
            if True:
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
        elif event == "-IMPORT_FLIGHT_PLAN-":
            filename = values['-FLIGHT_PLAN_FILE_PATH-']
            if Path(filename).is_file():
                try:
                    with open(filename, "rt", encoding='utf-8') as f:
                        text = f.read()
                    window['-UPDATE_CONTENT-'].Update(text)
                except Exception as e:
                    sg.popup(f"Error: {e}", title="Error")
    window.close()
    if iodsim_process: iodsim_process.terminate()

if __name__ == '__main__':
    main_window()
