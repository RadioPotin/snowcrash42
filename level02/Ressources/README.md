# level02

## Hint

There is a `level02.pcap` file in the home directory of user `level02`.
We will now turn to our pcap file reading tools to proceed, either:
- Wireshark
- tcpdump

After reading the output of `tcpdump -r level02.pcap`, it's obvious that we need a more modern and user friendly tool for analysing the packets captured in this file.

## scp

Scp binary will allow us to extract from our remote environment the relevant file in order to run Wireshark on it and inspect it further:

`scp -P 4242 scp://level02@192.168.152.166/level02.pcap level02.pcap`

So, since I have launched this command from my current level `Ressources` directory, the `level02.pcap` file is now available on my host machine.

## wireshark

By following the TCP stream that this pcap file holds we can see the following data:
```
..%..%..&..... ..#..'..$..&..... ..#..'..$.. .....#.....'........... .38400,38400....#.SodaCan:0....'..DISPLAY.SodaCan:0......xterm.........."........!........"..".....b........b....	B.
..............................1.......!.."......"......!..........."........"..".............	..
.....................
Linux 2.6.38-8-generic-pae (::ffff:10.1.1.2) (pts/10)

..wwwbugs login: l.le.ev.ve.el.lX.X
..
Password: ft_wandr...NDRel.L0L
.
..
Login incorrect
wwwbugs login:
```
This seemingly unreadable data infact holds the password necessary for the completion of the current level...

If we look at the lines

`..wwwbugs login: l.le.ev.ve.el.lX.X`

and

`Password: ft_wandr...NDRel.L0L`

We can see login and password prompts.
So this vulnerability seems to be that the passwords are not encrypted on the network and therefor accessible to whomever would happen to capture the given packets.

There is still something odd though. If we try to log in the VM with that exact password, it does not work. There is surely more to the characters we see than meets the eye...

## What is the deal with that `.` character ?

Just by reading the login prompt line we can easily deduce it to be that of a `backspace` input.

This new piece of information can help us derive the password from the raw data available to us.

`Password: ft_wandr...NDRel.L0L`

becomes

`Password: ft_waNDReL0L`

and indeed, using this derived password, we just so happen to log into the remote machine.
