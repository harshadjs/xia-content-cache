#!/bin/bash

name=$1

if [ "$name" = "" ]; then
	echo "Usage: $0 <name(alice/bob/...)>"
	exit 0
fi

echo "./peer.py ${name} peers.conf ${name}_has_chunk master_chunk_file"
./peer.py ${name} peers.conf ${name}_has_chunk master_chunk_file
