#!/usr/bin/python2
import sys
import time
import os

# find the path to xia-core
XIADIR=os.getcwd()
while os.path.split(XIADIR)[1] != 'xia-core':
    XIADIR=os.path.split(XIADIR)[0]
sys.path.append(XIADIR + '/api/lib')

import math
import socket
import struct
import time
import datetime
import string
from c_xsocket import *
from ctypes import *
import hashlib

sys.path.append(os.getcwd() + "/..")
from xia_address import *

class config_parser:
    def __init__(self):
        self.lines = []
        self.my_name = ""
        self.my_config = ""
        self.ad = ""
        self.hid = ""
        self.i4id = ""

    ## Read configuration file, Assumes that my_name has been set already
    def read_config_file(self, filename):
        with open(filename) as f:
            self.lines = f.readlines()

        for line in self.lines:
            ## ['alice', 'www-name-of-xia-sid', 'SID:xiasid']
            tokens = line.split(' ', 2)
            if tokens[0] == self.my_name:
                self.my_config = tokens

    ## Set my name
    def set_my_name(self, name):
        self.my_name = name

    ## Set ad
    def set_ad(self, ad):
        self.ad = ad

    ## Set hid
    def set_hid(self, hid):
        self.hid = hid

    ## Set 4id
    def set_i4id(self, i4id):
        self.i4id = i4id

    def get_ad(self):
        return self.ad

    def get_hid(self):
        return self.hid

    def get_my_dag(self):
        listen_dag = "RE %s %s %s" % (self.ad, self.hid, self.my_config[2])
        return listen_dag

    def get_my_name(self):
        return self.my_name


def construct_request_dag(chunkhash):
    dag = "RE ( %s %s ) CID:%s" % (config.get_ad(), config.get_hid(), chunkhash)
    return dag

## Help command: Lists all available commands
def cmd_help():
    print("Following commands are available: ")
    print("help             Displays this help.")
    print("list             Lists all available chunks.")
    print("get <chunkhash>  Gets chunk with chunkhash from somebody.")
    print("quit             Quit the application")


## List command: Lists all available commands
def cmd_list():
    pass

## Get command: Gets chunks from somebody
def cmd_get():
    chunkhash = raw_input("Enter chunkhash to get: ")

    cid_dag = construct_request_dag(chunkhash)
    print cid_dag
    sock = Xsocket(XSOCK_CHUNK)
    print("Got socket")
    XrequestChunk(sock, cid_dag)
    print("Requested")
    while True:
        chunk_status = XgetChunkStatus(sock, cid_dag)
        if chunk_status == READY_TO_READ:
            break
        time.sleep(1)

def xia_publish_chunks(filename):
    global cache_ctx

    cache_ctx = XallocCacheSlice(POLICY_DEFAULT, 0, 200000)
    with open(filename) as f:
        content_hashes = f.readlines()

    for content_hash in content_hashes:
        XputFile(cache_ctx, "./chunks/chunk_%s" % content_hash.lstrip().rstrip(), 512)


def establish_connections_xia():
    config.read_config_file(sys.argv[2])

    listen_sock = Xsocket(SOCK_STREAM)
    if listen_sock < 0:
        print("Can not open socket")

    (my_ad, my_hid, my_4id) = XreadLocalHostAddr(listen_sock)
    config.set_ad(my_ad)
    config.set_hid(my_hid)
    config.set_i4id(my_4id)

    Xbind(listen_sock, config.get_my_dag())
    print("Bind success to dag: " + config.get_my_dag())

    XregisterName(config.get_my_name(), config.get_my_dag())
    print("Name registered")

    xia_publish_chunks(sys.argv[3])

def main():
    print("***********************************************")
    print("Welcome to P2P file transfer program. You are: " + sys.argv[1])
    print("***********************************************")
    global my_ad, my_hid, my_sid, my_4id, config

    config = config_parser()
    config.set_my_name(sys.argv[1])

    establish_connections_xia()

    while True:
        answer = raw_input("p2p-peer > ")
        if answer == "help":
            cmd_help()
        elif answer == "get":
            cmd_get()
        elif answer == "list":
            cmd_list()
        elif answer == "quit":
            quit()

## Main routine ##
main()
