#!/usr/bin/env python

#Author:Gurucharan MK


import os, sys, stat


initScriptTemplate = '''#!/bin/bash

# absolute path to executable binary
progpath=<PATH_TO_EXEC>

# arguments to script
opts='-rpn'

# binary program name
prog=$(basename $progpath)

# pid file
pidfile="/var/run/${prog}.pid"

# make sure full path to executable binary is found
! [ -x $progpath ] && echo "$progpath: executable not found" && exit 1

eval_cmd() {
  local rc=$1
  if [ $rc -eq 0 ]; then
    echo '[  OK  ]'
  else
    echo '[FAILED]'
  fi
  return $rc
}

start() {
  # see if running
  local pids=$(pidof $prog)

  if [ -n "$pids" ]; then
    echo "$prog (pid $pids) is already running"
    return 0
  fi
  printf "%-50s%s" "Starting $prog: " ''
  $progpath $opts &

  # save pid to file if you want
  echo $! > $pidfile

  # check again if running
  pidof $prog >/dev/null 2>&1
  eval_cmd $?
}

stop() {
  # see if running
  local pids=$(pidof $prog)

  if [ -z "$pids" ]; then
    echo "$prog not running"
    return 0
  fi
  printf "%-50s%s" "Stopping $prog: " ''
  rm -f $pidfile
  kill -9 $pids
  eval_cmd $?
}

status() {
  # see if running
  local pids=$(pidof $prog)

  if [ -n "$pids" ]; then
    echo "$prog (pid $pids) is running"
  else
    echo "$prog is stopped"
  fi
}

case $1 in
  start)
    start
    ;;
  stop)
    stop
    ;;
  status)
    status
    ;;
  restart)
    stop
    sleep 1
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|status|restart}"
    exit 1
esac

exit $?
'''


execName = 'NonBlockingTCPServer'

completePath = os.path.join(os.getcwd(), execName)
initScriptTemplateMod = initScriptTemplate.replace(r'<PATH_TO_EXEC>', completePath)
  
initScriptFile = file('NonBlockingTCPServ_D', 'w')
initScriptFile.write(initScriptTemplateMod)
initScriptFile.close()

os.system('sudo cp NonBlockingTCPServ_D /etc/init.d/')
os.system('sudo chmod +rx /etc/init.d/NonBlockingTCPServ_D')
 
os.system('sudo update-rc.d NonBlockingTCPServ_D defaults')

