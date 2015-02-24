#!/bin/zsh

tmux new-window -n "term"
tmux new-window -n "click"
tmux new-window -c "$(pwd)/applications/example/" -n "app"
tmux split-window -c "$(pwd)/applications/example/"
tmux select-window -t 0
tmux new-window -c "$(pwd)/xcache/" -n "xcache" -d
tmux rename-window -t 0 "term"
