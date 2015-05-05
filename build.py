#!/usr/bin/env python

#Author:Gurucharan MK


import os

installLibEventCommand = 'sudo apt-get install libevent-dev'

os.system(installLibEventCommand)

compileCommand = 'sudo gcc -O2 NonBlockingTCPServer.c -levent -o NonBlockingTCPServer -std=c99'

os.system(compileCommand)
