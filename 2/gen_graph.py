import matplotlib.pyplot as plt
import csv

# Функция для чтения данных из CSV файла
def read_csv(filename):
    num_proc = []
    exec_time = []
    final_cost = []
    
    with open(filename, 'r') as file:
        reader = csv.DictReader(file)
        for row in reader:
            num_proc.append(int(row["num_proc"]))
            exec_time.append(float(row["avg_exec_time"]))
            final_cost.append(float(row["avg_final_cost"]))
    
    return num_proc, exec_time, final_cost

# Функция для построения и сохранения графиков
def plot_metrics(num_proc, exec_time, final_cost):
    plt.figure(figsize=(10, 8))

    # График времени выполнения
    plt.subplot(2, 1, 1)
    plt.plot(num_proc, exec_time, marker='o', linestyle='-', color='b')
    plt.title("Execution Time by Number of Processes")
    plt.xlabel("Number of Processes")
    plt.ylabel("Execution Time (seconds)")

    # График итоговой стоимости
    plt.subplot(2, 1, 2)
    plt.plot(num_proc, final_cost, marker='o', linestyle='-', color='g')
    plt.title("Final Cost by Number of Processes")
    plt.xlabel("Number of Processes")
    plt.ylabel("Final Cost")

    plt.tight_layout()
    
    # Сохранение графика в PNG
    plt.savefig("metrics_plot.png", format="png")
    # plt.show()

# Основной блок программы
def main():
    # Чтение данных из файла
    filename = "results_mult.csv"
    num_proc, exec_time, final_cost = read_csv(filename)
    
    # Построение и сохранение графиков
    plot_metrics(num_proc, exec_time, final_cost)

if __name__ == "__main__":
    main()
