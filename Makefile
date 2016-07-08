voltage_server: main.c
	gcc main.c -o voltage_server

voltage: voltage.c
	gcc voltage.c -o voltage

install: voltage_server main.c voltage
	cp voltage_server /usr/local/bin/voltage_server
	cp voltage_server_startup /etc/init.d/voltage_server
	chmod a+x /etc/init.d/voltage_server
	update-rc.d voltage_server defaults

uninstall:
	rm /usr/local/bin/voltage_server
	update-rc.d -f voltage_server remove
