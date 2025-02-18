if __name__ == "__main__":
    # Define the host and target point
    host = "http://127.0.0.1:18080"
    target_point = [50.0, 8.0, 27.0]

    # Create the environment
    env = IoDSimEnv(host, target_point)

    # Test the environment
    state, info = env.reset()
    print("Initial State:", state)

    done = False
    while not done:
        action = env.action_space.sample()  # Random action
        state, reward, done, truncated, info = env.step(action)
        print(f"Action: {action}, State: {state}, Reward: {reward}, Done: {done}")

    env.close()
