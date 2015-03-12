#!/bin/bash

count=5

rm -f chunks.hash
while [ $count != "0" ]; do
	dd if=/dev/urandom of=./tmpfile bs=512 count=1
	sha1sum=`sha1sum ./tmpfile | cut -d " " -f1`
	echo $sha1sum >> chunks.hash
	mv tmpfile "chunk_"${count}
	count=`expr $count - 1`
done
