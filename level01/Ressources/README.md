# level01

## Hint

No hint this time, after roaming around the file system, it occured that the `/etc/passwd` directory could be of use in order to get information on the current user, `level01`.

## cat

By printing the contents of the `/etc/passwd` we can notice outstanding information on our current target `flag01`.

```shell-session
level01@SnowCrash:~$ cat /etc/passwd
root:x:0:0:root:/root:/bin/bash
[...]
level00:x:2000:2000::/home/user/level00:/bin/bash
level01:x:2001:2001::/home/user/level01:/bin/bash
level02:x:2002:2002::/home/user/level02:/bin/bash
[...]
level14:x:2014:2014::/home/user/level14:/bin/bash
flag00:x:3000:3000::/home/flag/flag00:/bin/bash
flag01:42hDRfypTqqnw:3001:3001::/home/flag/flag01:/bin/bash
flag02:x:3002:3002::/home/flag/flag02:/bin/bash
[...]
flag14:x:3014:3014::/home/flag/flag14:/bin/bash
```

The format of `passwd` files is as follows:

```shell
mark:x:1001:1001:mark,,,:/home/mark:/bin/bash
[--] - [--] [--] [-----] [--------] [--------]
|    |   |    |     |         |        |
|    |   |    |     |         |        +-> 7. Login shell
|    |   |    |     |         +----------> 6. Home directory
|    |   |    |     +--------------------> 5. GECOS
|    |   |    +--------------------------> 4. GID
|    |   +-------------------------------> 3. UID
|    +-----------------------------------> 2. Password
+----------------------------------------> 1. Username
```

The password is quite obviously displayed, all there is left to do is to decrypt it.

To do this we use:

## John

John, or John the Ripper, is a tool to bruteforce weak passwords from `passwd` files.

Since we're only interested in the password for the `flag01` user, we can simply recreate a `passwd` file with a single line `flag01:42hDRfypTqqnw:3001:3001::/home/flag/flag01:/bin/bash` and feed to john as follows:

```shell-session
$ sudo john passwd
Loaded 1 password hash (descrypt, traditional crypt(3) [DES 128/128 SSE2-16])
Will run 8 OpenMP threads
Press 'q' or Ctrl-C to abort, almost any other key for status
abcdefg          (flag01)
1g 0:00:00:00 100% 2/3 16.66g/s 556500p/s 556500c/s 556500C/s 123456..thebest3
Use the "--show" option to display all of the cracked passwords reliably
Session completed
```

Then display it with:
```shell-session
$ sudo john --show passwd
flag01:abcdefg:3001:3001::/home/flag/flag01:/bin/bash
```

# Getflag

We can now log into the VM as user `flag01` and use password `abcdefg`.

Using the `getflag` command will now provide us the token allowing us to proceed to the next level!

```shell-session
flag01@192.168.152.166's password:
Don't forget to launch getflag !
flag01@SnowCrash:~$ getflag
Check flag.Here is your token : f2av5il02puano7naaf6adaaf
```
