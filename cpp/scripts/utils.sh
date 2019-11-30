# !/bin/sh
#

set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

# $1: the prefix this log will use as "[prefix] xxxx"
use_red_green_echo() {
  prefix="$1"
  red() {
    echo "$(tput bold)$(tput setaf 1)[$prefix] $*$(tput sgr0)";
  }
  
  green() {
    echo "$(tput bold)$(tput setaf 2)[$prefix] $*$(tput sgr0)";
  }
  
  yellow() {
    echo "$(tput bold)$(tput setaf 3)[$prefix] $*$(tput sgr0)";
  }
}


# $@: script that run on Mac ONLY
utils_on_mac(){
  if [[ $(uname -a) == *Darwin* ]]; then
    _utils_yellow "[ON-MAC][SUB-SHELL] $@"
    ("$@")
    _utils_yellow "Done"
  fi
}

# $@: script that run on Linux ONLY
utils_on_linux(){
  if [[ $(uname -a) == Linux* ]]; then
    _utils_yellow "[ON-LINUX][SUB-SHELL] $@"
    ("$@")
    _utils_yellow "Done"
  fi
}

