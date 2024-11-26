import requests

url = "http://127.0.0.1:18080/change_position"

def change_drone_position(drone_id, acceleration, time):

    headers = {'Content-Type': 'application/json'}
    payload = {
        "drone_id": drone_id,
        "acceleration": acceleration,
        "time": time
    }

    try:
        response = requests.post(url, json=payload, headers=headers)
        response.raise_for_status()  # Raise an error for bad status codes
        print("Response:", response.json())
    except requests.exceptions.RequestException as e:
        print("Error:", e)