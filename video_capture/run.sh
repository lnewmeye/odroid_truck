#use command 'tmux' to start a detachable session (so ssh can be shut down)
#user 'tmux attach' to restart session
export LD_LIBRARY_PATH=/usr/local/lib
./$1
