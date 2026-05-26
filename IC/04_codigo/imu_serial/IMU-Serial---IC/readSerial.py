#https://github.com/ReRob-USP/ExoGUI/blob/main/threads/threadXsensGaitPhase.hpp

import serial as s
import time
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import csv

# Configuração do arquivo CSV
csvfile = open('data.csv', 'w', newline='')
csvwriter = csv.writer(csvfile)
csvwriter.writerow(['time', 'ax', 'ay', 'az', 'gx', 'gy', 'gz'])


# Conexão serial
esp32 = s.Serial('COM3', 115200)

# Janela principal
root = tk.Tk()
root.title("Dados do ESP32 - Acelerômetro")

# Label para mostrar os valores
label = tk.Label(root, text="Esperando dados...")
label.pack()

# Configura o gráfico
fig, ax = plt.subplots(figsize=(5, 3))
bars = ax.bar(['ax', 'ay', 'az','gx', 'gy', 'gz'], [0, 0, 0,0,0,0])
ax.set_ylim(-20, 20)  # Ajuste conforme os limites do seu sensor

# Embute o gráfico na interface Tkinter
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()

# Loop de leitura e atualização
for _ in range(10000):
    line = esp32.readline().decode('utf-8', errors = 'ignore').strip()
    try:
        vec = line.split(',')
        if len(vec) < 7:
            continue
        floats = [float(i) for i in vec]
        [t, ax_val, ay_val, az_val, gx, gy, gz] = floats
        #concatenate in global array anssave in csv file
        csvwriter.writerow([time.time(), ax_val, ay_val, az_val, gx, gy, gz])
        csvfile.flush()
        

        # Atualiza o label
        label.config(text=f"{time.time():.2f} {floats}")

        # Atualiza as barras do gráfico (somente ax, ay, az)
        for bar, val in zip(bars, floats[1:7]):
            bar.set_height(val)
        
        if _%10 == 0:
            # Atualiza o gráfico
            canvas.draw()
            canvas.flush_events()

        # Atualiza a interface
        root.update()
    except Exception as e:
        print("Erro ao ler/parsing:", e)

esp32.close()
csvfile.close()