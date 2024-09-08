#!/usr/bin/python
import os
import matplotlib.pyplot as plt
import csv
from numpy import mean, sqrt, var

# Ruta del directorio de datos y salida
data_dir = 'data'
output_dir = 'images'
output_subdir  = ''
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

        max_latencies = []
        min_latencies = []
        avg_latencies = []
        payload_sizes = []

        max_latencies_stressed = []
        min_latencies_stressed = []
        avg_latencies_stressed = []
        payload_sizes_stressed = []

        for file in files:
            if file.endswith('.txt'):
                # Obtener el tamaño de payload y número de mensajes del nombre del archivo
                payload_size = int(file.split('-')[0].split('_')[1])
                nmsg = int(file.split('-')[1].split('_')[1].split('.')[0])
                is_stressed = 'stressed' in file

                # Leer los datos del archivo
                latencies = []
                repetitions = []

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

                except FileNotFoundError:
                    print(f"Error: El archivo '{file_path}' no se encontró.")
                    continue
                except ValueError:
                    print(f"Error: El archivo '{file_path}' contiene datos inválidos.")
                    continue

                # Calcular estadísticas
                latencies_x = []
                for i in range(len(latencies)):
                    latencies_x += [latencies[i]] * repetitions[i] # Repito valor de latencia segun cantidad de repeticiones obtenidas

                media = round(mean(latencies_x), 2)
                sigma2 = round(var(latencies_x), 2)
                sigma = round(sqrt(sigma2), 2)
                max_latency = max(latencies_x)
                min_latency = min(latencies_x)

                # Crear y guardar el histograma
                plt.figure(figsize=(10, 6))
                plt.stem(latencies, repetitions)
                plt.xlabel('Latency (µs)')
                plt.ylabel('Repetitions')
                plt.title(f'Latency Histogram {test_id}: BSize {bsize} - Payload {payload_size} - NMsg {nmsg} - Sigma {sigma} µs')
                plt.grid(axis='y', linestyle='--', alpha=0.7)
                plt.xticks(range(0, max(latencies) + 1, 50))  # Ajustar x-ticks si es necesario
                plt.xlim(media - 200, media + 200)
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
                results.append([test_id, bsize_int, payload_size, nmsg, 'Yes' if is_stressed else 'No', media, max_latency, min_latency, sigma2, sigma])
                test_id += 1

                if is_stressed:
                    max_latencies_stressed.append(max_latency)
                    min_latencies_stressed.append(min_latency)
                    avg_latencies_stressed.append(media)
                    payload_sizes_stressed.append(payload_size)
                else:
                    max_latencies.append(max_latency)
                    min_latencies.append(min_latency)
                    avg_latencies.append(media)
                    payload_sizes.append(payload_size)

        # sort results (avg, max, min) by payload size
        avg_latencies, max_latencies, min_latencies, payload_sizes = zip(*sorted(zip(avg_latencies, max_latencies, min_latencies, payload_sizes)))

        # Archivos de subcarpeta procesados
        fig, (ax1, ax2) = plt.subplots(2, 1, sharey=True, sharex=True)
        fig.suptitle(f'Latency vs Payload Size - BSize: {bsize}')
        ax1.set_title('Normal')
        ax1.set_xscale('log', base=2)
        ax1.stem(payload_sizes, avg_latencies, markerfmt='ro', linefmt='r-')
        ax1.stem(payload_sizes, max_latencies, markerfmt='go', linefmt='g-')
        ax1.stem(payload_sizes, min_latencies, markerfmt='bo', linefmt='b-')
        ax1.set(ylabel='Latency (µs)')
        ax1.grid(axis='y', linestyle='--', alpha=0.7)
        ax1.set(xlim=(2, max(payload_sizes)*5))

        if (len(payload_sizes_stressed) > 0):
            ax2.set_title('Stressed')
            ax2.set_xscale('log', base=2)
            ax2.stem(payload_sizes_stressed, avg_latencies_stressed, markerfmt='ro', linefmt='r-')
            ax2.stem(payload_sizes_stressed, max_latencies_stressed, markerfmt='go', linefmt='g-')
            ax2.stem(payload_sizes_stressed, min_latencies_stressed, markerfmt='bo', linefmt='b-')
            ax2.set(xlabel='Payload Size (Bytes)', ylabel='Latency (µs)')
            ax2.legend(['Avg', 'Max', 'Min'], loc='lower right')
            ax2.grid(axis='y', linestyle='--', alpha=0.7)

        fig.savefig(os.path.join(output_subdir, "latency_vs_payload.png"))
        plt.close(fig)

# Guardar los resultados en un archivo CSV
with open(csv_output, 'w', newline='') as csvfile:
    csvwriter = csv.writer(csvfile)
    csvwriter.writerow(['Test ID', 'BSize', 'PayloadSize', 'NMsg', 'Stressed', 'Avg Latency [µs]', 'Max Latency [µs]', 'Min Latency [µs]', 'Sigma2 [µs^2]', 'Sigma [µs]'])
    csvwriter.writerows(results)

print(f"Procesamiento completado. Resultados guardados en '{csv_output}'.")
