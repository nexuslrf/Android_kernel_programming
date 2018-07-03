## Files in Prom1:
* `ptree.c` : the source code for ptree module.
* `Makefile` :  configurations for make ptree. It will generate our target file `ptree.ko`.  **Note**: you need to change the `KID` to your local kernel directory!

## Files in Prom2:
* `test_for_ptree.c`: the source code for test our system call in Prom1.
* `Android.mk`: ndk-build configurations. It will generate a executable file `ptreeARM`.
* `testscript.txt`: shows the result of `ptreeARM`

## Files in Prom3:
* `my_process.c`: the source code using `fork()` and `execl()` to show hierarchy of parent and child processes.
* `Android.mk`: ndk-build configurations. It will generate a executable file `myProcARM`.
* `testscript.txt`: shows the result of `myProcARM`

## Files in Prom4:
* `server.c` : source code for server. **Note**: use `gcc server.c -lpthread` to compile.
* `client.c`: source code for client. **Note**: use `gcc server.c -lpthread` to compile.

