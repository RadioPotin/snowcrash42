# level12

## Hint

The first thing we can see is ... nothing. No file, no directory corresponding to either `flagXX` or `leveXX`.

So after a bit of digging in the whole system, only one thing remained untouched.
The binary getflag.

The first thing we did after learning how to use gdb and how we could see syscalls used in a binary, was indeed, to dig into getflag.

What does `ltrace` tells us:

```shell-session
__libc_start_main(0x8048946, 1, 0xbffff7e4, 0x8048ed0, 0x8048f40 <unfinished ...>
ptrace(0, 0, 1, 0, 0)                                                                                                                              = -1
puts("You should not reverse this"You should not reverse this
)                                                                                                                = 28
+++ exited (status 1) +++
```

This is puzzling, but since we have only been learning about how to do stuff we `should not` be doing, it doesn't seem like we should acknowledge this string "You should not reverse this", quite the contrary, this seems like the perfect place to search for vulnerabilities.


## First thoughts

What `ltrace` tells us.

First we can see that `ptrace` is used at the top of everything... But why is the message that tells us to `not reverse this binary` called explictly when we try to debug it ?

Let's try one more thing, gdb.

```shell-session
level14@SnowCrash:~$ gdb getflag
GNU gdb (Ubuntu/Linaro 7.4-2012.04-0ubuntu2.1) 7.4-2012.04
Copyright (C) 2012 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "i686-linux-gnu".
For bug reporting instructions, please see:
<http://bugs.launchpad.net/gdb-linaro/>...
Reading symbols from /bin/getflag...(no debugging symbols found)...done.
(gdb) b main
Breakpoint 1 at 0x804894a
(gdb) run
Starting program: /bin/getflag

Breakpoint 1, 0x0804894a in main ()
(gdb) n
Single stepping until exit from function main,
which has no line number information.
You should not reverse this
0xb7e454d3 in __libc_start_main () from /lib/i386-linux-gnu/libc.so.6
(gdb)
```

Same thing ! Again, we are warned to not reverse program on this binary.

### ptrace

After some research we found that `ltrace`, `gdb`, `strace` and cie are all using `ptrace()`, furthermore it cannot be called more than once in a given process...

And wrapping a binary with gdb produces a `single` process... So it is impossible to debug a program that calls `ptrace()` with `gdb` without having to deal with that issue.

What's `ptrace` ?

It is a system call that monitors memory and registers.

It's indeed convenient when you try to debug a program.

So it's seems that `ptrace()` is triggering something in `getflag` when it is used by the caller process. So we cannot go further.

We call that a *stripped binary*.

We must find a way to go deeper in our debug to find a possible solution and obtain our flag.

## Attempt 1

We know that `ptrace()` returns `-1` when failing.

So we thought about using a custom `ptrace()` to stub the one called in gdb and be able to pass through the call to it in `getflag`.

The env variable `LD_PRELOAD` may be the answer.

We then created a custom `ptrace` library as follow that always returns `0`.

```C
int ptrace(int *a, int*b, void *c)
{
    write(2, "Hello from my custom ptrace()\n", 30);
    return (0);
}
```

We compile it as a shared library so we can use this custom `ptrace` at runtime.

```shell-session
level14@SnowCrash:/tmp$ gcc -shared ptrace.c -o ptrace.so
```
And we start to use gdb. (In quiet mode)

```shell-session
level14@SnowCrash:/tmp$ gdb -q getflag
Reading symbols from /bin/getflag...(no debugging symbols found)...done.
(gdb)
```

We then set our custom `LD_PRELOAD `to the environment

```shell-session
(gdb) set environment LD_PRELOAD=/tmp/ptrace.so
```

And we launch our `getflag`.

```shell-session
(gdb) file getflag
Reading symbols from /bin/getflag...(no debugging symbols found)...done.
```

After setting up our breakpoint on main and run the debugger. We got an output !

```shell-session
(gdb) b main
Breakpoint 1 at 0x804894a
(gdb) run
Starting program: /bin/getflag

Breakpoint 1, 0x0804894a in main ()
(gdb) c
Continuing.
Hello from my custom ptrace()
Injection Linked lib detected exit..
[Inferior 1 (process 32106) exited with code 01]
(gdb)
The program is not being run.
(gdb)
```

Oups ! It seems like we cannot do that.

The program checks for `LD_PRELOAD` and exits it immediately.

That's was a nice try but it's not the way.


## Attempt 2 

So our custom `ptrace` wasn't working. Let's try an other way.

We must pass throught `ptrace` so let's see what's our disassembler in `gdb` shows us.

*NB: We got rid of our custom library*

```shell-session
(gdb) disas main
Dump of assembler code for function main:
   0x08048946 <+0>:	push   %ebp
   0x08048947 <+1>:	mov    %esp,%ebp
   0x08048949 <+3>:	push   %ebx
   0x0804894a <+4>:	and    $0xfffffff0,%esp
   0x0804894d <+7>:	sub    $0x120,%esp
   0x08048953 <+13>:	mov    %gs:0x14,%eax
   0x08048959 <+19>:	mov    %eax,0x11c(%esp)
   0x08048960 <+26>:	xor    %eax,%eax
   0x08048962 <+28>:	movl   $0x0,0x10(%esp)
   0x0804896a <+36>:	movl   $0x0,0xc(%esp)
   0x08048972 <+44>:	movl   $0x1,0x8(%esp)
   0x0804897a <+52>:	movl   $0x0,0x4(%esp)
   0x08048982 <+60>:	movl   $0x0,(%esp)
   0x08048989 <+67>:	call   0x8048540 <ptrace@plt>
   0x0804898e <+72>:	test   %eax,%eax
   0x08048990 <+74>:	jns    0x80489a8 <main+98>
   0x08048992 <+76>:	movl   $0x8048fa8,(%esp)
   0x08048999 <+83>:	call   0x80484e0 <puts@plt>
   0x0804899e <+88>:	mov    $0x1,%eax
   0x080489a3 <+93>:	jmp    0x8048eb2 <main+1388>
   0x080489a8 <+98>:	movl   $0x8048fc4,(%esp)
   0x080489af <+105>:	call   0x80484d0 <getenv@plt>
   0x080489b4 <+110>:	test   %eax,%eax
   0x080489b6 <+112>:	je     0x80489ea <main+164>
   0x080489b8 <+114>:	mov    0x804b040,%eax
   0x080489bd <+119>:	mov    %eax,%edx
   0x080489bf <+121>:	mov    $0x8048fd0,%eax
   0x080489c4 <+126>:	mov    %edx,0xc(%esp)
   0x080489c8 <+130>:	movl   $0x25,0x8(%esp)
   0x080489d0 <+138>:	movl   $0x1,0x4(%esp)
   0x080489d8 <+146>:	mov    %eax,(%esp)
   0x080489db <+149>:	call   0x80484c0 <fwrite@plt>
   0x080489e0 <+154>:	mov    $0x1,%eax
   0x080489e5 <+159>:	jmp    0x8048eb2 <main+1388>
   0x080489ea <+164>:	movl   $0x0,0x4(%esp)
   0x080489f2 <+172>:	movl   $0x8048ff6,(%esp)
   0x080489f9 <+179>:	call   0x8048500 <open@plt>
   0x080489fe <+184>:	test   %eax,%eax
   0x08048a00 <+186>:	jle    0x8048a34 <main+238>
   0x08048a02 <+188>:	mov    0x804b040,%eax
   0x08048a07 <+193>:	mov    %eax,%edx
   0x08048a09 <+195>:	mov    $0x8048fd0,%eax
   0x08048a0e <+200>:	mov    %edx,0xc(%esp)
   0x08048a12 <+204>:	movl   $0x25,0x8(%esp)
   0x08048a1a <+212>:	movl   $0x1,0x4(%esp)
   0x08048a22 <+220>:	mov    %eax,(%esp)
   0x08048a25 <+223>:	call   0x80484c0 <fwrite@plt>
   0x08048a2a <+228>:	mov    $0x1,%eax
   0x08048a2f <+233>:	jmp    0x8048eb2 <main+1388>
   0x08048a34 <+238>:	movl   $0x0,0x4(%esp)
   0x08048a3c <+246>:	movl   $0x8049009,(%esp)
   0x08048a43 <+253>:	call   0x804871c <syscall_open>
   0x08048a48 <+258>:	mov    %eax,0x14(%esp)
   0x08048a4c <+262>:	cmpl   $0xffffffff,0x14(%esp)
   0x08048a51 <+267>:	jne    0x8048e88 <main+1346>
   0x08048a57 <+273>:	mov    0x804b040,%eax
   0x08048a5c <+278>:	mov    %eax,%edx
   0x08048a5e <+280>:	mov    $0x804901c,%eax
---Type <return> to continue, or q <return> to quit---
   0x08048a63 <+285>:	mov    %edx,0xc(%esp)
   0x08048a67 <+289>:	movl   $0x46,0x8(%esp)
   0x08048a6f <+297>:	movl   $0x1,0x4(%esp)
   0x08048a77 <+305>:	mov    %eax,(%esp)
   0x08048a7a <+308>:	call   0x80484c0 <fwrite@plt>
   0x08048a7f <+313>:	mov    $0x1,%eax
   0x08048a84 <+318>:	jmp    0x8048eb2 <main+1388>
   0x08048a89 <+323>:	movl   $0x8049063,0x4(%esp)
   0x08048a91 <+331>:	lea    0x1c(%esp),%eax
   0x08048a95 <+335>:	mov    %eax,(%esp)
   0x08048a98 <+338>:	call   0x8048843 <isLib>
   0x08048a9d <+343>:	test   %eax,%eax
   0x08048a9f <+345>:	je     0x8048aae <main+360>
   0x08048aa1 <+347>:	movl   $0x1,0x10(%esp)
   0x08048aa9 <+355>:	jmp    0x8048e89 <main+1347>
   0x08048aae <+360>:	cmpl   $0x0,0x10(%esp)
   0x08048ab3 <+365>:	je     0x8048e89 <main+1347>
   0x08048ab9 <+371>:	movl   $0x8049068,0x4(%esp)
   0x08048ac1 <+379>:	lea    0x1c(%esp),%eax
   0x08048ac5 <+383>:	mov    %eax,(%esp)
   0x08048ac8 <+386>:	call   0x8048843 <isLib>
   0x08048acd <+391>:	test   %eax,%eax
   0x08048acf <+393>:	je     0x8048e46 <main+1280>
   0x08048ad5 <+399>:	mov    0x804b060,%eax
   0x08048ada <+404>:	mov    %eax,%edx
   0x08048adc <+406>:	mov    $0x804906c,%eax
   0x08048ae1 <+411>:	mov    %edx,0xc(%esp)
   0x08048ae5 <+415>:	movl   $0x20,0x8(%esp)
   0x08048aed <+423>:	movl   $0x1,0x4(%esp)
   0x08048af5 <+431>:	mov    %eax,(%esp)
   0x08048af8 <+434>:	call   0x80484c0 <fwrite@plt>
   0x08048afd <+439>:	call   0x80484b0 <getuid@plt>
   0x08048b02 <+444>:	mov    %eax,0x18(%esp)
   0x08048b06 <+448>:	mov    0x18(%esp),%eax
   0x08048b0a <+452>:	cmp    $0xbbe,%eax
   0x08048b0f <+457>:	je     0x8048ccb <main+901>
   0x08048b15 <+463>:	cmp    $0xbbe,%eax
   0x08048b1a <+468>:	ja     0x8048b68 <main+546>
   0x08048b1c <+470>:	cmp    $0xbba,%eax
   0x08048b21 <+475>:	je     0x8048c3b <main+757>
   0x08048b27 <+481>:	cmp    $0xbba,%eax
   0x08048b2c <+486>:	ja     0x8048b4d <main+519>
   0x08048b2e <+488>:	cmp    $0xbb8,%eax
   0x08048b33 <+493>:	je     0x8048bf3 <main+685>
   0x08048b39 <+499>:	cmp    $0xbb8,%eax
   0x08048b3e <+504>:	ja     0x8048c17 <main+721>
   0x08048b44 <+510>:	test   %eax,%eax
   0x08048b46 <+512>:	je     0x8048bc6 <main+640>
   0x08048b48 <+514>:	jmp    0x8048e06 <main+1216>
   0x08048b4d <+519>:	cmp    $0xbbc,%eax
   0x08048b52 <+524>:	je     0x8048c83 <main+829>
   0x08048b58 <+530>:	cmp    $0xbbc,%eax
   0x08048b5d <+535>:	ja     0x8048ca7 <main+865>
   0x08048b63 <+541>:	jmp    0x8048c5f <main+793>
   0x08048b68 <+546>:	cmp    $0xbc2,%eax
   0x08048b6d <+551>:	je     0x8048d5b <main+1045>
   0x08048b73 <+557>:	cmp    $0xbc2,%eax
   0x08048b78 <+562>:	ja     0x8048b95 <main+591>
   0x08048b7a <+564>:	cmp    $0xbc0,%eax
---Type <return> to continue, or q <return> to quit---
   0x08048b7f <+569>:	je     0x8048d13 <main+973>
   0x08048b85 <+575>:	cmp    $0xbc0,%eax
   0x08048b8a <+580>:	ja     0x8048d37 <main+1009>
   0x08048b90 <+586>:	jmp    0x8048cef <main+937>
   0x08048b95 <+591>:	cmp    $0xbc4,%eax
   0x08048b9a <+596>:	je     0x8048da3 <main+1117>
   0x08048ba0 <+602>:	cmp    $0xbc4,%eax
   0x08048ba5 <+607>:	jb     0x8048d7f <main+1081>
   0x08048bab <+613>:	cmp    $0xbc5,%eax
   0x08048bb0 <+618>:	je     0x8048dc4 <main+1150>
   0x08048bb6 <+624>:	cmp    $0xbc6,%eax
   0x08048bbb <+629>:	je     0x8048de5 <main+1183>
   0x08048bc1 <+635>:	jmp    0x8048e06 <main+1216>
   0x08048bc6 <+640>:	mov    0x804b060,%eax
   0x08048bcb <+645>:	mov    %eax,%edx
   0x08048bcd <+647>:	mov    $0x8049090,%eax
   0x08048bd2 <+652>:	mov    %edx,0xc(%esp)
   0x08048bd6 <+656>:	movl   $0x21,0x8(%esp)
   0x08048bde <+664>:	movl   $0x1,0x4(%esp)
   0x08048be6 <+672>:	mov    %eax,(%esp)
   0x08048be9 <+675>:	call   0x80484c0 <fwrite@plt>
   0x08048bee <+680>:	jmp    0x8048e2f <main+1257>
   0x08048bf3 <+685>:	mov    0x804b060,%eax
   0x08048bf8 <+690>:	mov    %eax,%ebx
   0x08048bfa <+692>:	movl   $0x80490b2,(%esp)
   0x08048c01 <+699>:	call   0x8048604 <ft_des>
   0x08048c06 <+704>:	mov    %ebx,0x4(%esp)
   0x08048c0a <+708>:	mov    %eax,(%esp)
   0x08048c0d <+711>:	call   0x8048530 <fputs@plt>
   0x08048c12 <+716>:	jmp    0x8048e2f <main+1257>
   0x08048c17 <+721>:	mov    0x804b060,%eax
   0x08048c1c <+726>:	mov    %eax,%ebx
   0x08048c1e <+728>:	movl   $0x80490cc,(%esp)
   0x08048c25 <+735>:	call   0x8048604 <ft_des>
   0x08048c2a <+740>:	mov    %ebx,0x4(%esp)
   0x08048c2e <+744>:	mov    %eax,(%esp)
   0x08048c31 <+747>:	call   0x8048530 <fputs@plt>
   0x08048c36 <+752>:	jmp    0x8048e2f <main+1257>
   0x08048c3b <+757>:	mov    0x804b060,%eax
   0x08048c40 <+762>:	mov    %eax,%ebx
   0x08048c42 <+764>:	movl   $0x80490e6,(%esp)
   0x08048c49 <+771>:	call   0x8048604 <ft_des>
   0x08048c4e <+776>:	mov    %ebx,0x4(%esp)
   0x08048c52 <+780>:	mov    %eax,(%esp)
   0x08048c55 <+783>:	call   0x8048530 <fputs@plt>
   0x08048c5a <+788>:	jmp    0x8048e2f <main+1257>
   0x08048c5f <+793>:	mov    0x804b060,%eax
   0x08048c64 <+798>:	mov    %eax,%ebx
   0x08048c66 <+800>:	movl   $0x8049100,(%esp)
   0x08048c6d <+807>:	call   0x8048604 <ft_des>
   0x08048c72 <+812>:	mov    %ebx,0x4(%esp)
   0x08048c76 <+816>:	mov    %eax,(%esp)
   0x08048c79 <+819>:	call   0x8048530 <fputs@plt>
   0x08048c7e <+824>:	jmp    0x8048e2f <main+1257>
   0x08048c83 <+829>:	mov    0x804b060,%eax
   0x08048c88 <+834>:	mov    %eax,%ebx
   0x08048c8a <+836>:	movl   $0x804911a,(%esp)
   0x08048c91 <+843>:	call   0x8048604 <ft_des>
   0x08048c96 <+848>:	mov    %ebx,0x4(%esp)
---Type <return> to continue, or q <return> to quit---
   0x08048c9a <+852>:	mov    %eax,(%esp)
   0x08048c9d <+855>:	call   0x8048530 <fputs@plt>
   0x08048ca2 <+860>:	jmp    0x8048e2f <main+1257>
   0x08048ca7 <+865>:	mov    0x804b060,%eax
   0x08048cac <+870>:	mov    %eax,%ebx
   0x08048cae <+872>:	movl   $0x8049134,(%esp)
   0x08048cb5 <+879>:	call   0x8048604 <ft_des>
   0x08048cba <+884>:	mov    %ebx,0x4(%esp)
   0x08048cbe <+888>:	mov    %eax,(%esp)
   0x08048cc1 <+891>:	call   0x8048530 <fputs@plt>
   0x08048cc6 <+896>:	jmp    0x8048e2f <main+1257>
   0x08048ccb <+901>:	mov    0x804b060,%eax
   0x08048cd0 <+906>:	mov    %eax,%ebx
   0x08048cd2 <+908>:	movl   $0x804914e,(%esp)
   0x08048cd9 <+915>:	call   0x8048604 <ft_des>
   0x08048cde <+920>:	mov    %ebx,0x4(%esp)
   0x08048ce2 <+924>:	mov    %eax,(%esp)
   0x08048ce5 <+927>:	call   0x8048530 <fputs@plt>
   0x08048cea <+932>:	jmp    0x8048e2f <main+1257>
   0x08048cef <+937>:	mov    0x804b060,%eax
   0x08048cf4 <+942>:	mov    %eax,%ebx
   0x08048cf6 <+944>:	movl   $0x8049168,(%esp)
   0x08048cfd <+951>:	call   0x8048604 <ft_des>
   0x08048d02 <+956>:	mov    %ebx,0x4(%esp)
   0x08048d06 <+960>:	mov    %eax,(%esp)
   0x08048d09 <+963>:	call   0x8048530 <fputs@plt>
   0x08048d0e <+968>:	jmp    0x8048e2f <main+1257>
   0x08048d13 <+973>:	mov    0x804b060,%eax
   0x08048d18 <+978>:	mov    %eax,%ebx
   0x08048d1a <+980>:	movl   $0x8049182,(%esp)
   0x08048d21 <+987>:	call   0x8048604 <ft_des>
   0x08048d26 <+992>:	mov    %ebx,0x4(%esp)
   0x08048d2a <+996>:	mov    %eax,(%esp)
   0x08048d2d <+999>:	call   0x8048530 <fputs@plt>
   0x08048d32 <+1004>:	jmp    0x8048e2f <main+1257>
   0x08048d37 <+1009>:	mov    0x804b060,%eax
   0x08048d3c <+1014>:	mov    %eax,%ebx
   0x08048d3e <+1016>:	movl   $0x804919c,(%esp)
   0x08048d45 <+1023>:	call   0x8048604 <ft_des>
   0x08048d4a <+1028>:	mov    %ebx,0x4(%esp)
   0x08048d4e <+1032>:	mov    %eax,(%esp)
   0x08048d51 <+1035>:	call   0x8048530 <fputs@plt>
   0x08048d56 <+1040>:	jmp    0x8048e2f <main+1257>
   0x08048d5b <+1045>:	mov    0x804b060,%eax
   0x08048d60 <+1050>:	mov    %eax,%ebx
   0x08048d62 <+1052>:	movl   $0x80491b6,(%esp)
   0x08048d69 <+1059>:	call   0x8048604 <ft_des>
   0x08048d6e <+1064>:	mov    %ebx,0x4(%esp)
   0x08048d72 <+1068>:	mov    %eax,(%esp)
   0x08048d75 <+1071>:	call   0x8048530 <fputs@plt>
   0x08048d7a <+1076>:	jmp    0x8048e2f <main+1257>
   0x08048d7f <+1081>:	mov    0x804b060,%eax
   0x08048d84 <+1086>:	mov    %eax,%ebx
   0x08048d86 <+1088>:	movl   $0x80491d0,(%esp)
   0x08048d8d <+1095>:	call   0x8048604 <ft_des>
   0x08048d92 <+1100>:	mov    %ebx,0x4(%esp)
   0x08048d96 <+1104>:	mov    %eax,(%esp)
   0x08048d99 <+1107>:	call   0x8048530 <fputs@plt>
   0x08048d9e <+1112>:	jmp    0x8048e2f <main+1257>
---Type <return> to continue, or q <return> to quit---
   0x08048da3 <+1117>:	mov    0x804b060,%eax
   0x08048da8 <+1122>:	mov    %eax,%ebx
   0x08048daa <+1124>:	movl   $0x80491ea,(%esp)
   0x08048db1 <+1131>:	call   0x8048604 <ft_des>
   0x08048db6 <+1136>:	mov    %ebx,0x4(%esp)
   0x08048dba <+1140>:	mov    %eax,(%esp)
   0x08048dbd <+1143>:	call   0x8048530 <fputs@plt>
   0x08048dc2 <+1148>:	jmp    0x8048e2f <main+1257>
   0x08048dc4 <+1150>:	mov    0x804b060,%eax
   0x08048dc9 <+1155>:	mov    %eax,%ebx
   0x08048dcb <+1157>:	movl   $0x8049204,(%esp)
   0x08048dd2 <+1164>:	call   0x8048604 <ft_des>
   0x08048dd7 <+1169>:	mov    %ebx,0x4(%esp)
   0x08048ddb <+1173>:	mov    %eax,(%esp)
   0x08048dde <+1176>:	call   0x8048530 <fputs@plt>
   0x08048de3 <+1181>:	jmp    0x8048e2f <main+1257>
   0x08048de5 <+1183>:	mov    0x804b060,%eax
   0x08048dea <+1188>:	mov    %eax,%ebx
   0x08048dec <+1190>:	movl   $0x8049220,(%esp)
   0x08048df3 <+1197>:	call   0x8048604 <ft_des>
   0x08048df8 <+1202>:	mov    %ebx,0x4(%esp)
   0x08048dfc <+1206>:	mov    %eax,(%esp)
   0x08048dff <+1209>:	call   0x8048530 <fputs@plt>
   0x08048e04 <+1214>:	jmp    0x8048e2f <main+1257>
   0x08048e06 <+1216>:	mov    0x804b060,%eax
   0x08048e0b <+1221>:	mov    %eax,%edx
   0x08048e0d <+1223>:	mov    $0x8049248,%eax
   0x08048e12 <+1228>:	mov    %edx,0xc(%esp)
   0x08048e16 <+1232>:	movl   $0x38,0x8(%esp)
   0x08048e1e <+1240>:	movl   $0x1,0x4(%esp)
   0x08048e26 <+1248>:	mov    %eax,(%esp)
   0x08048e29 <+1251>:	call   0x80484c0 <fwrite@plt>
   0x08048e2e <+1256>:	nop
   0x08048e2f <+1257>:	mov    0x804b060,%eax
   0x08048e34 <+1262>:	mov    %eax,0x4(%esp)
   0x08048e38 <+1266>:	movl   $0xa,(%esp)
   0x08048e3f <+1273>:	call   0x8048520 <fputc@plt>
   0x08048e44 <+1278>:	jmp    0x8048ead <main+1383>
   0x08048e46 <+1280>:	movl   $0x8049281,0x4(%esp)
   0x08048e4e <+1288>:	lea    0x1c(%esp),%eax
   0x08048e52 <+1292>:	mov    %eax,(%esp)
   0x08048e55 <+1295>:	call   0x80487be <afterSubstr>
   0x08048e5a <+1300>:	test   %eax,%eax
   0x08048e5c <+1302>:	jne    0x8048e89 <main+1347>
   0x08048e5e <+1304>:	mov    0x804b040,%eax
   0x08048e63 <+1309>:	mov    %eax,%edx
   0x08048e65 <+1311>:	mov    $0x8049294,%eax
   0x08048e6a <+1316>:	mov    %edx,0xc(%esp)
   0x08048e6e <+1320>:	movl   $0x30,0x8(%esp)
   0x08048e76 <+1328>:	movl   $0x1,0x4(%esp)
   0x08048e7e <+1336>:	mov    %eax,(%esp)
   0x08048e81 <+1339>:	call   0x80484c0 <fwrite@plt>
   0x08048e86 <+1344>:	jmp    0x8048ead <main+1383>
   0x08048e88 <+1346>:	nop
   0x08048e89 <+1347>:	mov    0x14(%esp),%eax
   0x08048e8d <+1351>:	mov    %eax,0x8(%esp)
   0x08048e91 <+1355>:	movl   $0x100,0x4(%esp)
   0x08048e99 <+1363>:	lea    0x1c(%esp),%eax
   0x08048e9d <+1367>:	mov    %eax,(%esp)
---Type <return> to continue, or q <return> to quit---
   0x08048ea0 <+1370>:	call   0x804874c <syscall_gets>
   0x08048ea5 <+1375>:	test   %eax,%eax
   0x08048ea7 <+1377>:	jne    0x8048a89 <main+323>
   0x08048ead <+1383>:	mov    $0x0,%eax
   0x08048eb2 <+1388>:	mov    0x11c(%esp),%edx
   0x08048eb9 <+1395>:	xor    %gs:0x14,%edx
   0x08048ec0 <+1402>:	je     0x8048ec7 <main+1409>
   0x08048ec2 <+1404>:	call   0x80484a0 <__stack_chk_fail@plt>
   0x08048ec7 <+1409>:	mov    -0x4(%ebp),%ebx
   0x08048eca <+1412>:	leave
   0x08048ecb <+1413>:	ret
End of assembler dump.
```


There is a lot going on...

First, let's find ptrace().

```shell-session
Dump of assembler code for function main:
   0x08048946 <+0>:	push   %ebp
   0x08048947 <+1>:	mov    %esp,%ebp
   0x08048949 <+3>:	push   %ebx
   0x0804894a <+4>:	and    $0xfffffff0,%esp
   0x0804894d <+7>:	sub    $0x120,%esp
   0x08048953 <+13>:	mov    %gs:0x14,%eax
   0x08048959 <+19>:	mov    %eax,0x11c(%esp)
   0x08048960 <+26>:	xor    %eax,%eax
   0x08048962 <+28>:	movl   $0x0,0x10(%esp)
   0x0804896a <+36>:	movl   $0x0,0xc(%esp)
   0x08048972 <+44>:	movl   $0x1,0x8(%esp)
   0x0804897a <+52>:	movl   $0x0,0x4(%esp)
   0x08048982 <+60>:	movl   $0x0,(%esp)
   0x08048989 <+67>:	call   0x8048540 <ptrace@plt>     <----- Call to ptrace
   0x0804898e <+72>:	test   %eax,%eax
   0x08048990 <+74>:	jns    0x80489a8 <main+98>
```

It's here ! We can see that the function call of ptrace executes and there is a test after.

Let's see what's there.

```shell-session
(gdb) b *0x0804898e
Breakpoint 1 at 0x804898e
(gdb) run
Starting program: /bin/getflag

Breakpoint 1, 0x0804898e in main ()
(gdb) print $eax
$1 = -1
```

After printing the value we can conclude that we got kicked out right there, the return value is `-1`.

And if we can set it to `0` ?

```shell-session
(gdb) set $eax=0
(gdb) print $eax
$2 = 0
```

Let's continue.

```shell-session
(gdb) c
Continuing.
Check flag.Here is your token :
Nope there is no token here for you sorry. Try again :)
[Inferior 1 (process 32143) exited normally]
(gdb)
```

We succesfully pass through `ptrace` and we have the correct output, i.e. without the error message !

But we don't have the token yet...

With `disas main` we can see that there is a call to `getuid()`, as seen in previous exercises. 

Let's see what's happening there:

We start over with the same settings than before. And let's add an other breakpoint to the stackframe with cmp. It must be there. The check for the user uid.

```shell-session
level14@SnowCrash:/tmp$ gdb -q getflag
Reading symbols from /bin/getflag...(no debugging symbols found)...done.
(gdb) b *0x0804898e
Breakpoint 1 at 0x804898e
(gdb) b *0x08048b0a
Breakpoint 2 at 0x8048b0a
(gdb) run
Starting program: /bin/getflag

Breakpoint 1, 0x0804898e in main ()
(gdb) set $eax=1
(gdb) n
Single stepping until exit from function main,
which has no line number information.

Breakpoint 2, 0x08048b0a in main ()
(gdb) print $eax
$1 = 2014
```

Yes ! We can see that we have our current user uid `2014` referring to `level14` !

We should be getting something different if we set an `UID` corresponding to another user.

For example, UID `3014`, that as we can see in the `/etc/passwd` file, corresponding to the user `flag14`.

## getflag

```shell-session
(gdb) set $eax=3014
(gdb) c
Continuing.
Check flag.Here is your token : 7QiHafiNa3HVozsaXkawuYrTstxbpABHD8CPnHJ
[Inferior 1 (process 32189) exited normally]
```

Easy !
