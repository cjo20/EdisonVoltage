It is possible to read the Lipo battery voltage on an Intel Edison without using the Yocto battery_voltage. This method of reading battery voltage works on devices running Ubilinux.

There is a system device, bcove_adc, that is connected to various voltage and temperature input lines. These can be read using `/sys/devices/platform/bcove_adc/basincove_gpadc`. 

* `/sys/devices/platform/bcove_adc/basincove_gpadc/channel` reads a bitmask of channels to report. Channel 0 is the battery voltage.
* `/sys/devices/platform/bcove_adc/basincove_gpadc/sample` triggers an ADC conversion to take place.
* `/sys/devices/platform/bcove_adc/basincove_gpadc/result` contains the result of the ADC conversion. It is an integer between 0 and 1023. A result of 1023 is roughly equivalent to 4.5v.

```
$ echo 0x1 | sudo tee /sys/devices/platform/bcove_adc/basincove_gpadc/channel
1
$ echo 1 | sudo tee /sys/devices/platform/bcove_adc/basincove_gpadc/sample
1
$ cat /sys/devices/platform/bcove_adc/basincove_gpadc/result
sample_result[0] = 980
sample_result[1] = 0
sample_result[2] = 0
sample_result[3] = 0
sample_result[4] = 0
sample_result[5] = 0
sample_result[6] = 0
sample_result[7] = 0
sample_result[8] = 0
```

In this case, the result is 980. 980 is about 96% of 1023, so the voltage is about 96% of 4.5v. This is 4.3v. In this case, the battery is connected to the charger, so this value is to be expected.


There are two different methods of reading voltage provided. One, standalone.c, requires root access (must be run with `sudo`). Build it with `make standalone`. standalone.c takes 1 argument when run, a number. The number dictates the output format.
* `./standalone 0` or `./standalone` : Full text output
* `./standalone 1` CSV output
* `./standalone 2` produces results in the format `100% 4300mV`

The other method, main.c, is a service which watches for accesses to files much like sysfs. These files are set up to be accessable by all users, so once the server process has started, no further root access is required. 
To start the server, run :
```
make voltage_server
sudo ./voltage_server &
```

The interface for this method is located in `/tmp`. To trigger an ADC read, write `1` to /tmp/battery_trigger

```
$ echo 1 > /tmp/battery_trigger
```

After some length of time (0.1 seconds is sufficient), the results are available in 3 files:

1. `/tmp/battery_voltage` contains the voltage in mV
2. `/tmp/battery_percentage` contains the battery percentage.
3. `/tmp/battery_raw` contains the average of 5 ADC readings.

The python script, report_voltages.py, gives an example of how to use this interface.

Running `sudo make install` will install `voltage_server` as a service and start automatically when the edison starts. This is thoroughly untested.

For OpenAPS users that want to log voltage to nightscout, I use the following two files:
https://gist.github.com/cjo20/42ce5227cc1412513448da17f5f1ab84
