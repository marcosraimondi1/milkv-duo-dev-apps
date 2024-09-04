#!/usr/bin/python
import os
import matplotlib.pyplot as plt
import csv

# Ruta del directorio de datos y salida
data_dir = 'data'
output_dir = 'images'
csv_output = 'test_results.csv'

# Crear lista para almacenar los resultados
results = []

# Inicializar el contador de Test ID
test_id = 1

# Crear directorio de salida si no existe
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Iterar sobre las subcarpetas en el directorio de datos
for subdir, _, files in os.walk(data_dir):
    if os.path.basename(subdir).startswith('bsize'):
        bsize = os.path.basename(subdir)

        for file in files:
            if file.endswith('.txt'):
                # Obtener el tamaño de payload y número de mensajes del nombre del archivo
                payload_size = file.split('-')[0].split('_')[1]
                nmsg = file.split('-')[1].split('_')[1].split('.')[0]
                
                # Leer los datos del archivo
                latencies = []
                repetitions = []
                total_latency = 0
                total_repetitions = 0
                max_latency = 0
                min_latency = float('inf')

                file_path = os.path.join(subdir, file)
                try:
                    with open(file_path, 'r') as f:
                        # Omitir la línea de encabezado
                        next(f)
                        for line in f:
                            latency, repetition = map(int, line.split(','))
                            if repetition <= 0:
                                continue
                            latencies.append(latency)
                            repetitions.append(repetition)

                            # Calcular estadísticas
                            total_latency += latency * repetition
                            total_repetitions += repetition
                            max_latency = max(max_latency, latency)
                            min_latency = min(min_latency, latency)
                except FileNotFoundError:
                    print(f"Error: El archivo '{file_path}' no se encontró.")
                    continue
                except ValueError:
                    print(f"Error: El archivo '{file_path}' contiene datos inválidos.")
                    continue

                # Calcular promedio de latencia
                avg_latency = total_latency / total_repetitions if total_repetitions > 0 else 0
                avg_latency = round(avg_latency, 4)

                # Crear y guardar el gráfico
                plt.figure(figsize=(10, 6))
                plt.stem(latencies, repetitions)
                plt.xlabel('Latency (µs)')
                plt.ylabel('Repetitions')
                plt.title(f'Latency Histogram {file}')
                plt.grid(axis='y', linestyle='--', alpha=0.7)
                plt.xticks(range(0, max(latencies) + 1, 50))  # Ajustar x-ticks si es necesario
                plt.xlim(avg_latency - 200, avg_latency + 200)
                plt.tight_layout()

                # Crear carpeta de salida específica si no existe
                output_subdir = os.path.join(output_dir, bsize)
                if not os.path.exists(output_subdir):
                    os.makedirs(output_subdir)

                # Guardar el gráfico
                plt.savefig(os.path.join(output_subdir, file.replace('.txt', '.png')))
                plt.close()

                # Agregar resultados a la lista
                bsize_int = bsize.split('e')[1]
                results.append([test_id, '', bsize_int, payload_size, nmsg, avg_latency, max_latency, min_latency])
                test_id += 1

# Guardar los resultados en un archivo CSV
with open(csv_output, 'w', newline='') as csvfile:
    csvwriter = csv.writer(csvfile)
    csvwriter.writerow(['Test ID', 'Title', 'BSize', 'PayloadSize', 'NMsg', 'Avg Latency [µs]', 'Max Latency [µs]', 'Min Latency [µs]'])
    csvwriter.writerows(results)

print(f"Procesamiento completado. Resultados guardados en '{csv_output}'.")
