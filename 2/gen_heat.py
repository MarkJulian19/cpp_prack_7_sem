import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def generate_heatmaps(file_path="results.csv"):
    # Чтение данных из CSV файла
    df = pd.read_csv(file_path)
    
    # Уникальные методы охлаждения
    cooling_methods = df["cooling_method"].unique()

    for method in cooling_methods:
        # Фильтрация данных по текущему методу охлаждения
        method_data = df[df["cooling_method"] == method]

        # Создание сводной таблицы для тепловой карты из execution_time
        heatmap_data_execution = method_data.pivot_table(index="num_processors", columns="num_jobs", values="execution_time", aggfunc='mean')

        # Построение тепловой карты из execution_time
        plt.figure(figsize=(10, 8))
        sns.heatmap(heatmap_data_execution, annot=True, fmt=".2f", cmap="YlOrRd", cbar_kws={'label': 'Execution Time (s)'})
        plt.gca().invert_yaxis()  # Инвертирование оси Y
        plt.title(f"Heatmap of Execution Time for {method.capitalize()} Cooling")
        plt.xlabel("Number of Jobs")
        plt.ylabel("Number of Processors")
        plt.tight_layout()

        # Сохранение тепловой карты в файл
        plt.savefig(f"{method}_cooling_heatmap_execution_time.png")
        plt.close()
        print(f"Heatmap for {method} cooling method saved as {method}_cooling_heatmap_execution_time.png")

        # Создание тепловых карт из final_cost
        heatmap_data_final_cost = method_data.pivot_table(index="num_processors", columns="num_jobs", values="final_cost", aggfunc='mean')

        # Построение тепловой карты final_cost
        plt.figure(figsize=(10, 8))
        sns.heatmap(heatmap_data_final_cost, annot=True, fmt=".2f", cmap="Blues", cbar_kws={'label': 'Final Cost'})
        plt.gca().invert_yaxis()  # Инвертирование оси Y
        plt.title(f"Heatmap of Final Cost for {method.capitalize()} Cooling")
        plt.xlabel("Number of Jobs")
        plt.ylabel("Number of Processors")
        plt.tight_layout()

        # Сохранение тепловой карты в файл
        plt.savefig(f"{method}_cooling_heatmap_final_cost.png")
        plt.close()
        print(f"Heatmap for {method} cooling method saved as {method}_cooling_heatmap_final_cost.png")

# Запуск функции для генерации картинок
generate_heatmaps("results.csv")
