import csv
import random

def generate_csv_output(num_jobs, min_duration, max_duration, output_file="jobs.csv"):
    """
    Генерирует CSV файл с данными о работах.
    """
    with open(output_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Запись заголовка
        writer.writerow(["Job ID", "Duration"])
        
        # Запись данных о работах
        for job_id in range(1, num_jobs + 1):
            duration = random.randint(min_duration, max_duration)
            writer.writerow([f"Job_{job_id}", duration])
    print(f"CSV data generated and saved to {output_file}")

def main():
    # Параметры генерации данных
    num_jobs = int(input("Enter the number of jobs: "))
    min_duration = int(input("Enter the minimum job duration: "))
    max_duration = int(input("Enter the maximum job duration: "))

    # Генерация CSV файла
    generate_csv_output(num_jobs, min_duration, max_duration)

if __name__ == "__main__":
    main()