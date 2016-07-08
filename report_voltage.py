#!/bin/python
from time import sleep

f = open('/tmp/battery_trigger', 'w')
f.write('1');
f.close();
sleep(0.1);
f = open('/tmp/battery_voltage', 'r')
voltage = f.read();
f.close();

f = open('/tmp/battery_percentage', 'r')
percentage = f.read();
f.close();

f = open('/tmp/battery_raw', 'r')
raw = f.read();
f.close();

print(voltage + "mV, " + percentage + "%, " + raw)

