Android Memory Analyzer
=====================

Breifing
---------------

This is a memory analyzer targeting android os, but it is portable to any linux system, or even a Windows system.
    It is used to analyze native heap usage, native heap leaking.

Usage
---------------

We first compile a shared library targeted to our android machine. Current version has support x86/arm android machine. The compile step is simple. Just press ndk-build.
    Now we have a share library in libs/\$TARGET_ARCH/libmemanaly.so. We push it into /data/local/tmp like this:
```
    adb push libs/\$TARGET_ARCH/libmemanaly.so /data/local/tmp/
```
Then we need to push the wrapper too.
```
adb push mywrapper.sh /data/local/tmp/
```
We set the mode of our wrapper.
```
adb shell chmod 755 /data/local/tmp/mywrapper.sh
```
Set the property of the machine, to tell Android start our application with this wrapper. Note our machine must have been rooted.
```
adb shell su -c "setprop wrap.<our application package name> /data/local/tmp/mywrapper.sh"
```
Note here our application package name has a length limit to 31 characters. If you have such a long package name ( normally a service of yours), just truncate it to 31 characters.
    Start our application. Let's rock for a while.

Now we are ready to pull the heap snapshot to our PC. Let's do this. Here I assume you are using a Linux OS, with all GNU tools.
    First we forward the port from the android machine to our PC.
```
    adb forward tcp:3244 tcp:3244
```
If you don't care about the leaking, you may do this
```
    adb forward tcp:3255 tcp:3255
```

3255 port transfers simpler data, but not suitable for analyzing leaking.
    We use nc utility to transfer the snapshot back.
```
    nc localhost 3244 >/tmp/snapshot
```
then we can do the analyze:
```
    python analyze.py -b /tmp/snapshot >/tmp/result
```
This command give the heap usage, the allocation elements is sorted descending.
    We usually care the top ten element, which would solve most bugs.
    And this command:
```
    python analyze.py -m /tmp/snapshot
```
Using mark and sweep algorithm to filter out the leaking. The result is stored at /tmp/analyze_zero.

All this reports have elements with backtracing information. But we have no idea what these backtracing information means, for they are all numbers.
    We filter our resulted report like this, make it readable:
```
    cat <resulted report> | python AddressFilter.py <maps file> <paths to your symbol files>
```
The maps file must read from your application process **THE SAME PROCESS AS YOU GET THE SNAPSHOT** .
    We get it lke this:
```
    adb shell su -c "cat /proc/<pid>/maps" >maps
```

**Just Enjoy It**.
