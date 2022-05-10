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
2. A token file we are not allowed to read of write to.

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

## Sending a file outward

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
Connecting to 127.0.0.1:6969 .. Connected!
Sending file .. wrote file!
```
Success !
But can we be sure ?

After some research there seems to be a tool we should use in order to check that this file transfer if A-OK: [netcat](https://linuxize.com/post/netcat-nc-command-with-examples/).

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

And indeed, when we launch the `nc` tool with the right `-l` option, and redo the sequence of commands we did for our first test we get:
```shell-session
level10@SnowCrash:~$ nc -l 6969
.*( )*.
yup
```

## Accessing the file

## getflag
