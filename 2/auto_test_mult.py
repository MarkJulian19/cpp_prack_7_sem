import subprocess
import time
import csv
import re

# Функция для запуска программы с параметром и получения результата
def run_main_mult(num_proc):
    try:
        start_time = time.time()  # Начало замера времени
        # Запуск программы с параметром num_proc
        result = subprocess.run(
            ['./main_mult.o', str(num_proc)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        end_time = time.time()  # Окончание замера времени
        
        # Извлечение времени выполнения
        exec_time = end_time - start_time
        
        # Поиск строки "Current best solution cost: 1483" с помощью регулярного выражения
        match = re.search(r'Current best solution cost: (\d+)', result.stdout)
        if match:
            final_cost = int(match.group(1))
            print(final_cost, num_proc)
        else:
            final_cost = None
        
        return exec_time, final_cost
    except Exception as e:
        print(f"Ошибка при выполнении: {e}")
        return None, None

# Запись результатов в CSV
def write_to_csv(filename, data):
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Запись заголовков
        writer.writerow(["num_proc", "avg_exec_time", "avg_final_cost"])
        # Запись данных
        writer.writerows(data)

# Основной блок программы
def main():
    data = []
    # Запуск тестирования 5 раз для каждого значения num_proc
    for num_proc in range(2, 15, 2):
        exec_times = []
        final_costs = []
        
        # Запуск программы 5 раз для получения среднего значения
        for _ in range(5):
            exec_time, final_cost = run_main_mult(num_proc)
            if exec_time is not None and final_cost is not None:
                exec_times.append(exec_time)
                final_costs.append(final_cost)

        # Рассчитываем среднее время выполнения и итоговую стоимость
        if exec_times and final_costs:
            avg_exec_time = sum(exec_times) / len(exec_times)
            avg_final_cost = sum(final_costs) / len(final_costs)
            print(avg_exec_time, avg_final_cost)
        else:
            avg_exec_time = None
            avg_final_cost = None

        # Добавляем результат в таблицу данных
        data.append([num_proc, avg_exec_time, avg_final_cost])

    # Запись всех результатов в CSV файл
    write_to_csv("results_mult.csv", data)
    print("Результаты записаны в results_mult.csv")

if __name__ == "__main__":
    main()
