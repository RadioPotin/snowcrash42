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

Another thing is that we **CANNOT** run binary on a file we do not have the rights over:
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
1. the `socket` system call is used to establish connection to host
2. the `access` system call is used to check rights before openng file

#### strace

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

This gives us a clear idea of what are the parameters given to the system calls we identified with `gdb`.

### reading manuals I

`socket` takes a `SOCK_STEAM` and the [man](https://man7.org/linux/man-pages/man2/socket.2.html) for this type of port to port communication reads:

>       SOCK_STREAM
>             Provides sequenced, reliable, two-way, connection-based
>             byte streams.  An out-of-band data transmission mechanism
>             may be supported.

The key word here is `two-way`, this must mean that if there is nobody listening the port the binary is trying to send the file to, then the system call fails.

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

### reading manuals II

The man for [access](https://man7.org/linux/man-pages/man2/access.2.html) eventually tells us:

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

So there is a security hole to exploit in that binary in order to send it outward

## getflag
