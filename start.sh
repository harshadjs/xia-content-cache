#!/bin/bash

router=1
nameserver=1

sudo ifconfig eth1 up
if [ "$router" = "1" ]; then
	sudo ifconfig eth2 up
fi

if [ "$router" = "1" ]; then
	if [ "$nameserver" = "1" ]; then
		flags="-r -n"
	else
		flags="-r"
	fi
else
	if [ "$nameserver" = "1" ]; then
		flags="-t -l eth1 -f eth0 -n"
	else
		flags="-t -l eth1 -f eth0"
	fi
fi

sudo ./bin/xianet ${flags} stop
sudo ./bin/xianet ${flags} start
