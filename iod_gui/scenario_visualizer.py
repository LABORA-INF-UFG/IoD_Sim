import time
import PySimpleGUI as sg
from controller import request_metrics_thread
from figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from threading import Thread
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TkAgg')

sg.theme('LightGrey1')
# common
update_figures_event_thread = None

def draw_figure(canvas, figure):
    if not hasattr(draw_figure, 'canvas_packed'):
        draw_figure.canvas_packed = {}
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    widget = figure_canvas_agg.get_tk_widget()
    if widget not in draw_figure.canvas_packed:
        draw_figure.canvas_packed[widget] = figure
        widget.pack(side='top', fill='both', expand=1)
    return figure_canvas_agg


def delete_figure_agg(figure_agg):
    figure_agg.get_tk_widget().forget()
    try:
        draw_figure.canvas_packed.pop(figure_agg.get_tk_widget())
    except Exception as e:
        print(f'Error removing {figure_agg} from list', e)
    plt.close('all')


def create_update_figure_event(window, sleep_time=0.5):
    while request_metrics_thread.is_alive():
        window.write_event_value('-UPDATE_FIG-', None)
        time.sleep(sleep_time)

def plot_fig(choice, window):
    figure = Figure(choice)
    figure.update()
    try:
        figure_agg = draw_figure(window['-CANVAS-'].TKCanvas, figure.fig)
    except Exception as e:
        print('Error in plotting', e)
    return figure_agg, figure

def get_choice(values):
    try:
        return values['-LISTBOX-'][0]
    except Exception as e:
        return "Flight Path"


def create_window():
    figure_w, figure_h = 640, 480
    # define the form layout
    listbox_values = ['Flight Path', 'Metrics']
    col_listbox = [[sg.Listbox(values=listbox_values, change_submits=True, size=(28, len(listbox_values)), key='-LISTBOX-')],
                   [sg.Input(visible=False, enable_events=True, key='-SAVE_AS-', expand_x=True), sg.FileSaveAs()]]

    col_canvas = sg.Col([[sg.Canvas(size=(figure_w, figure_h), key='-CANVAS-')]])

    layout = [[sg.Text('Scenario Visualizer')],
              [sg.Col(col_listbox), col_canvas], ]

    # create the form and show it without the plot
    window = sg.Window('Scenario Visualizer',
                    layout, resizable=True, finalize=True)

    request_metrics_thread.daemon = True
    request_metrics_thread.start()
    update_figures_event_thread = Thread(
        target=create_update_figure_event, args=(window, ), daemon=True)
    update_figures_event_thread.start()


    figure_agg = None
    figure = None
    choice = None
    last_choice = choice
    while True:
        event, values = window.read()
        if event in (sg.WIN_CLOSED, 'Exit'):
            break
        choice = get_choice(values)
        if event == "-UPDATE_FIG-" and last_choice == choice and figure_agg and figure and choice:
            figure.update()
            figure_agg.draw()
        elif last_choice != choice:
            if figure_agg:
                delete_figure_agg(figure_agg)
            last_choice = choice
            figure_agg, figure = plot_fig(choice, window)
        elif event == "-SAVE_AS-":
            if not figure:
                sg.popup(
                    f"Select a figure first.", title='Error')
            else:
                plt.savefig(values['-SAVE_AS-'],
                            dpi=600, bbox_inches='tight')
if __name__ == '__main__':
    create_window()
