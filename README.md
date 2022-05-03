# snowcrash42

[PDF](https://linx.zapashcanon.fr/qo9bz68s.pdf)

## CTF

The project is basically a Capture The Flag in which you connect to a Virtual Machine as a given user from `level00` to `level14` and each of these user has a corresponding `flagXX` you need to provide, as well as the method used to get it, as a series of directories in the final repository of the project.

Each flag directory will have a corresponding README to explain the method used to get it.

## ssh

`ssh levelXX@<IP> -p 4242`

`XX`->the current level in the CTF

`IP`->the IP display as a prompt of the VM

## Recommended ressources

MUST HAVE:
- Wireshark, to read pcap files
- gdb, GNU debugger
- John, active password cracking tool
- Python, for scripting and automating each exercise
- Perl, Lua, for other scripting tasks
- Shell scripting

External ressources:
- cloudshark.org
- www.dcode.fr
