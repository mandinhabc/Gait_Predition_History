import pandas as pd 
import matplotlib.pyplot as plt
import numpy as np


# Read the CSV file
df = pd.read_csv('data.csv')

print(df)

x = df['time']
yx = df['ax']
yy = df['ay']
yz = df['az']

zx = df['gx']
zy = df['gy']
zz = df['gz']


fig, ax = plt.subplots(2, 3, figsize=(12, 7))

ax[0,0].plot(x, yx, label='ax(m/s)', color='blue')
ax[0,0].grid()
ax[0,0].set_xlabel('time(s)')
ax[0,0].set_ylabel('AX (rad/s)')
ax[0,0].legend()


ax[0,1].plot(x, yy, label='ay(m/s)', color='green')
ax[0,1].grid()
ax[0,1].set_xlabel('time(s)')
ax[0,1].set_ylabel('AY (rad/s)')
ax[0,1].legend()


ax[0,2].plot(x, yz, label='az(m/s)', color='red')
ax[0,2].grid()
ax[0,2].set_xlabel('time(s)')
ax[0,2].set_ylabel('AZ (rad/s)')
ax[0,2].legend()

ax[1,0].plot(x, zx, label='gx(rad/s)', color='cyan')
ax[1,0].grid()
ax[1,0].set_xlabel('time(s)')
ax[1,0].set_ylabel('GX (rad/s)')
ax[1,0].legend()

ax[1,1].plot(x, zy, label='gy(rad/s)', color='#90EE90')
ax[1,1].grid()
ax[1,1].set_xlabel('time(s)')
ax[1,1].set_ylabel('GY (rad/s)')
ax[1,1].legend()

ax[1,2].plot(x, zz, label='gz(rad/s)', color='magenta')
ax[1,2].grid()
ax[1,2].set_xlabel('time(s)')
ax[1,2].set_ylabel('GZ (rad/s)')
ax[1,2].legend()


plt.tight_layout()
plt.show()