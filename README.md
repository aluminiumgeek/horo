#Horo#

Horo is a modular IRC bot written in C++.  
It is extremely fast, lightweight (takes only 2MiB of RAM) and extendable

###Requirements###

* libpcre-dev for regexp handling
* python-dev (at least 2.7) for python modules

###Building###

Script <code>start.sh</code> builds and runs Horo. Don't forget to declare host and port. 
Also make your changes in <code>config</code> file.

###Running###
Horo makes many processes with <code>fork()</code>. You can watch what each of processes doing with <code>ps cx</code> command in your terminal. 
Typical output of this command with explanation:

    1237 pts/9    S+     0:00 horo             | main process
    1246 pts/9    S+     0:00 horo:c/lorng.py  | process that calls core-module lorng.py at time interval
    1250 pts/9    S+     0:00 horo:m/fib.py    | process that waits for a response from standart module fib.py
    1253 pts/9    S+     0:00 horo:e/twit.py   | process that waits for a response from module twit.py called by an event
    1255 pts/9    S+     0:00 horo: url title  | process that gets title from a URL


###Modules###
All modules are written in Python. You can see a typical module structure in file <code>modules/test.py</code>  
[Modules repository](https://github.com/aluminiumgeek/horo-modules)

Put all modules, except modules from <code>core</code> directory, to the <code>modules</code> directory.  
Core-modules are called every <code>x</code> seconds. For each of core-module you need an entry in 'mod.conf' in a format:

    /path/to/module.py time_in_seconds.

Put modules from repo's <code>core</code> directory into the horo's <code>core</code> directory and change variables <code>CHANNEL</code> in modules source code.  
List of modules prints by <code>%lsmod</code> command on any channel.


[Why Horo?](http://animevice.com/holo/18-16320)


