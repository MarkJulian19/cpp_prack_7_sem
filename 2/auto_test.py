import csv
import random
import subprocess
import time

def generate_csv_output(num_jobs, min_duration, max_duration, output_file="jobs.csv"):
    """
    Генерирует CSV файл с данными о работах.
    """
    with open(output_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Job ID", "Duration"])
        for job_id in range(1, num_jobs + 1):
            duration = random.randint(min_duration, max_duration)
            writer.writerow([f"Job_{job_id}", duration])
    print(f"CSV data generated with {num_jobs} jobs and saved to {output_file}")

def run_simulation(filename, num_processors, cooling_method):
    """
    Запускает C++ программу с заданными параметрами и возвращает финальную стоимость и время выполнения.
    """
    total_cost = 0
    total_time = 0
    num_runs = 5

    for _ in range(num_runs):
        start_time = time.time()
        result = subprocess.run(
            ["./main_solo.o", filename, str(num_processors), cooling_method],
            capture_output=True,
            text=True
        )
        end_time = time.time()

        # Найти строку с "Best solution found with cost:" и получить значение стоимости
        output_lines = result.stdout.splitlines()
        final_cost = None
        for line in output_lines:
            if "Best solution found with cost:" in line:
                final_cost = float(line.split(":")[-1].strip())
                break

        # Если стоимость не найдена, вернем None и сообщение об ошибке
        if final_cost is None:
            print(f"Error in program output: {result.stdout}")
            return None, None

        total_cost += final_cost
        total_time += (end_time - start_time)

    average_cost = total_cost / num_runs
    average_time = total_time / num_runs

    return average_cost, average_time

def main():
    # Параметры для тестирования
    num_jobs_list = [256000, 128000, 64000, 32000, 16000]  # Различные значения количества работ
    min_duration = 1
    max_duration = 100
    num_processors_list = [640, 320, 160, 80, 40]
    cooling_methods = ["boltzmann", "cauchy", "logarithmic"]

    # Открываем файл для записи результатов
    with open("results.csv", mode="w", newline="") as results_file:
        writer = csv.writer(results_file)
        writer.writerow(["num_jobs", "num_processors", "cooling_method", "final_cost", "execution_time"])

        # Запускаем программу с различными параметрами
        for num_jobs in num_jobs_list:
            # Генерация файла CSV для текущего значения num_jobs
            generate_csv_output(num_jobs, min_duration, max_duration)

            for num_processors in num_processors_list:
                for cooling_method in cooling_methods:
                    print(f"Running with {num_jobs} jobs, {num_processors} processors, and {cooling_method} cooling")
                    average_cost, average_time = run_simulation("jobs.csv", num_processors, cooling_method)
                    if average_cost is not None:
                        writer.writerow([num_jobs, num_processors, cooling_method, average_cost, average_time])
                        print(f"Result: Jobs = {num_jobs}, Processors = {num_processors}, Cooling = {cooling_method}, Average Cost = {average_cost}, Average Time = {average_time:.2f} seconds")
                    else:
                        print("Error in simulation, skipping result")

if __name__ == "__main__":
    main()
