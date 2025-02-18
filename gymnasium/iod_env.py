import gymnasium as gym
from gymnasium import spaces
import requests, time
import numpy as np

class IoDSimEnv(gym.Env):
    def __init__(self, host, target_point):
        super(IoDSimEnv, self).__init__()

        # Simulator host
        self.host = host
        self.target_point = np.array(target_point)  # Target point as a numpy array

        # Define the action space
        self.action_space = spaces.Discrete(8)

        # Map integer actions to vectors
        self.action_mapping = {
            0: np.array([0, 0, 0]),
            1: np.array([0, 0, 1]),
            2: np.array([0, 1, 0]),
            3: np.array([0, 1, 1]),
            4: np.array([1, 0, 0]),
            5: np.array([1, 0, 1]),
            6: np.array([1, 1, 0]),
            7: np.array([1, 1, 1]),
        }

    def reset(self):
        # Restart the simulation
        self.stop()
        time.sleep(1)
        self.start()

        # Get initial state
        state = self._get_realtime_metrics()
        return state, {}

    def step(self, action):
        direction = self.action_mapping[action]
        
        # Send action to the simulator
        data = {
            "drone_id": 0,
            "acceleration": {
                "x": direction[0],
                "y": direction[1],
                "z": direction[2]
            },
            "time": 2
        }
        response = requests.post(f"{self.host}/change_position", json=data)
        if response.status_code != 200:
            raise RuntimeError("Failed to move the drone.")

        # Get new state
        state = self._get_realtime_metrics()

        # Compute reward and check if the episode is done
        reward, done = self._compute_reward(state)

        return state, reward, done, False, {}

    def _get_realtime_metrics(self):
        response = requests.get(f"{self.host}/get_realtime_metrics")
        if response.status_code == 200:
            metrics = response.json()
            return np.array([
                metrics["location"]["x"],
                metrics["location"]["y"],
                metrics["location"]["z"],
                metrics["battery"]
            ])
        else:
            raise RuntimeError("Failed to fetch realtime metrics.")

    def _compute_reward(self, state):
        position = state[:3]
        distance = np.linalg.norm(position - self.target_point)

        # Define a reward function (negative distance)
        reward = -distance

        # Consider the episode done if the drone is close enough to the target
        done = distance < 0.1
        return reward, done


    def start(self):
        # Start the simulation
        response = requests.post(f"{self.host}/simulation")
        if response.status_code != 200:
            raise RuntimeError("Failed to start the simulation.")

    def close(self):
        # Stop the simulation
        response = requests.delete(f"{self.host}/stop")
        if response.status_code != 200:
            raise RuntimeError("Failed to stop the simulation.")
