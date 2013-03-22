# About #
A shell ipmsg program based on curses, simple but meaningful.

# Library Used #
The code of the library is from my rep [clib](https://github.com/isayme/clib "https://github.com/isayme/clib"),  but I just used libudp.

# Build #
Just use command `make` will be ok.  
If you got a error like `cannot find -lcurses`, congratulations, you should try to edit the file `makefile`,
find and change the `-lcurses` to `-lncurses`.  
If Ok, you may want to have a look at 
[What's the difference between -lcurses and -lncurses when compiling C using ncurses lib?](http://stackoverflow.com/questions/1517756/whats-the-difference-between-lcurses-and-lncurses-when-compiling-c-using-ncur).  
If still not ok, I'm sorry, maybe your system have no curses installed~

# Screenshot #

## the screenshot of shell:  ##
![shell](https://raw.github.com/isayme/cipmsg/master/bin/cipmsg_1.png)  

## the screenshot of ipmsg:  ##
![ipmsg](https://raw.github.com/isayme/cipmsg/master/bin/cipmsg_2.png)  

# Contact #
Email : isaymeorg [at] gmail [dot] com  
Blog  : [www.isayme.org](www.isayme.org "www.isayme.org") [Chinese Simplified]
