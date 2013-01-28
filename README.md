Simple status bar writen with C. Speedly end save resources. [Screenshot](http://ompldr.org/vaDk3dw/fb.png)(right bottom).

If not exist swap - don't show.


Install
-------

	# build
	make
	# copy to /usr/local/bin/
	sudo make install
	# or to /usr/bin
	#PREFIX='/usr' make install


Usage
-----

add to you ~/.tmux.conf

	# Update interval (2 good choice)
	set -g status-interval 2
	# Right statusbar
	set -g status-right '#[fg=green][ #[fg=white]#(/usr/local/bin/tmux-sb) #[fg=green]|#[default] %H:%M#[fg=green] ]#[default]'

and you can see in tmux:

	[ CPU: 13% | MEM: 76% SWAP: 1% | 02:25 ]

