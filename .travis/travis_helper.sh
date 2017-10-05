#!/bin/bash

# travis_wait : tweaked to display output with progress as dot
# from (piwik/tests/PHPUnit/travis-helper.sh)

status_delay=60

travis_wait() {
  local timeout=60
  local cmd="$@"

  $cmd &
  local cmd_pid=$!

  travis_jigger $! $timeout $cmd &
  local jigger_pid=$!
  local result

  {
    wait $cmd_pid 2>/dev/null
    result=$?
    ps -p$jigger_pid &>/dev/null && kill $jigger_pid
  } || return 1

  if [ $result -eq 0 ]; then
    echo -e "\n${GREEN}Command \"$cmd\" exited with $result.${RESET}"
  else
    echo -e "\n${RED}Command \"$cmd\" exited with $result.${RESET}"
  fi

  return $result
}

travis_jigger() {
  # helper method for travis_wait()
  local cmd_pid=$1
  shift
  local timeout=60
  shift
  local count=0

  # clear the line
  echo -e "\n"

  while [ $count -lt $timeout ]; do
    count=$(($count + 1))
    echo -ne "."
    sleep $status_delay
  done

  echo -e "\n${RED}Timeout $(($timeout * $status_delay)) seconds reached. Terminating \"$@\"${RESET}\n"
  kill -9 $cmd_pid
}
