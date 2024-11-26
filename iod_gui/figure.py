from controller import delay_data, throughput_data, jitter_data, tx_power_data, battery_data, packet_loss_data, location_data, zsp_location_data
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TkAgg')

colors = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:gray',
          'tab:brown', 'tab:pink', 'tab:purple', 'tab:olive', 'tab:cyan']

lines = ['dashed', 'dotted', 'dashdot', (0, (3, 5, 1, 5)), (0, (5, 1)), (0, (3, 1, 1, 1)), (0, (3, 1, 1, 1, 1, 1))]

class Figure():
  def __init__(self, name):
    self.name = name
    self.base_station = False
    if name == "Flight Path":
      plt.rcParams.update({'font.size': 10})
      self.fig = plt.figure(figsize=(12, 8))
      self.ax = self.fig.add_subplot(111, projection='3d')
      self._format_flight_path()

    elif name == "Metrics":
      plt.rcParams.update({'font.size': 12})
      self.fig, self.ax = plt.subplots(3, 2, figsize=(12, 8), sharex='col')
      self._format_metrics
    else:
      return "Unsupported type"

  def _update_flight_path(self):
    self.ax.cla()
    for _, zsp_location_points in zsp_location_data.items():
        _, zsp_location_values = zip(*zsp_location_points)
        self.ax.scatter(zsp_location_values[-1]['x'], \
                        zsp_location_values[-1]['y'], \
                        zsp_location_values[-1]['z'], \
                        marker='^', s=100, color='black', label='Base Station')
    i = 1
    for id_value, location_points in location_data.items():
      _, location_values = zip(*location_points)

      x = [d['x'] for d in location_values[:-1]]
      y = [d['y'] for d in location_values[:-1]]
      z = [d['z'] for d in location_values[:-1]]
      self.ax.plot(x, y, z, '--', linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])
      self.ax.plot(location_values[-1]['x'], location_values[-1]['y'], location_values[-1]['z'], 'o-',
                   linestyle=lines[id_value], label=None, color=colors[id_value])

      i+=1
    self.ax.set_xlabel("x (m)")
    self.ax.set_ylabel("y (m)")
    self.ax.set_zlabel("z (m)")

    self._format_flight_path()

  def _update_metrics(self):
    self.ax[0, 0].cla()
    self.ax[0, 1].cla()
    self.ax[1, 0].cla()
    self.ax[1, 1].cla()
    self.ax[2, 0].cla()
    self.ax[2, 1].cla()

    line = '--'
    for id_value, delay_points in delay_data.items():
        time_values, delay_values = zip(*delay_points)
        self.ax[0, 0].plot(time_values, delay_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    for id_value, throughput_points in throughput_data.items():
        time_values, throughput_values = zip(*throughput_points)
        self.ax[0, 1].plot(time_values, throughput_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    for id_value, jitter_points in jitter_data.items():
        time_values, jitter_values = zip(*jitter_points)
        self.ax[1, 0].plot(time_values, jitter_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    for id_value, tx_power_points in tx_power_data.items():
        time_values, tx_power_values = zip(*tx_power_points)
        self.ax[1, 1].plot(time_values, tx_power_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    for id_value, battery_points in battery_data.items():
        time_values, battery_values = zip(*battery_points)
        self.ax[2, 0].plot(time_values, battery_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    for id_value, packet_loss_points in packet_loss_data.items():
        time_values, packet_loss_values = zip(*packet_loss_points)
        self.ax[2, 1].plot(time_values, packet_loss_values, line,
                           linestyle=lines[id_value], label=f'Drone {id_value}', color=colors[id_value])

    self._format_metrics()

  def _format_flight_path(self):
    self.ax.set_xlabel('X (m)')
    self.ax.set_ylabel('Y (m)')
    self.ax.set_zlabel('Z (m)')
    self.ax.legend()
    self.ax.grid(True)
    # plt.title("Drone's Flight Path")

  def _format_metrics(self):
    for i in range(0, 3):
      for j in range(0, 2):
        self.ax[i, j].legend()
        self.ax[i, j].grid(True)
    self.ax[0, 0].set_ylabel('Delay (ns)')
    self.ax[0, 1].set_ylabel('Throughput (kbps)')
    self.ax[1, 0].set_ylabel('Jitter (ns)')
    self.ax[1, 1].set_ylabel('TX Power (dBm)')
    self.ax[2, 0].set_ylabel('Battery (mAh)')
    self.ax[2, 1].set_ylabel('Packet loss (%)')
    self.ax[2, 0].set_xlabel('Time (s)')
    self.ax[2, 1].set_xlabel('Time (s)')
    # self.fig.suptitle("Scenario Metrics")


  def update(self):
    if self.name == "Flight Path":
      self._update_flight_path()
    elif self.name == "Metrics":
      self._update_metrics()
