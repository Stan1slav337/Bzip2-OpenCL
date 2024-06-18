import subprocess
import time

def run_app_and_measure_time(app_command, file_argument, flag):

    # Combine the app command and the file argument
    command = f"{app_command} {file_argument} {flag}"
    
    # Start the timer
    start_time = time.time()
    
    # Run the command
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    # Stop the timer
    end_time = time.time()
    
    # Check if the command was executed successfully
    if result.returncode == 0:
        print("Command executed successfully!")
        print("Output:", result.stdout.decode())
    else:
        print("Command failed!")
        print("Error:", result.stderr.decode())
    
    # Calculate the execution time
    execution_time = end_time - start_time
    
    return execution_time

# Example usage
for i in range(10):
    time_taken = run_app_and_measure_time("./app", "1.txt", "-k")
    print(f"Time taken: {time_taken} seconds")
