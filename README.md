# Linux-Process-Tree
A simple Linux kernel program that outputs the running process tree.
To add functionality to the code just type: ./pstree (flag1) (flag2) (arg1) (arg2)
    There are 5 flags: 
    -p (print pid) (ex: ./pstree -p -> prints out pstree with pid)
    -a (print all active process in proc), 
    -g (print pgid)
    -n (print specific pid as root) (ex: ./pstree -n 2 -> prints the pstree of process with pid 2)
    -s (sort the process alphabetically instead of by pid)
    Note: It is possible to combine most of the flags, example: ./pstree -p -a (prints all active process and print pid alongside it).
    Note that -a -n is mutually exclusive and the program will take the first flag that appears
    If there is no further argument, the program will print the default output.
