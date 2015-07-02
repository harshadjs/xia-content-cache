#!/bin/bash

router=1
nameserver=0

sudo ifconfig enp2s0 up
if [ "$router" = "1" ]; then
##	sudo ifconfig eth2 up
	echo ""
fi

if [ "$router" = "1" ]; then
	if [ "$nameserver" = "1" ]; then
		flags="-r -n"
	else
		flags="-r -vvvv"
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
