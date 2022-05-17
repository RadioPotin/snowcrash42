# level09

## Hint

1. As we log into the VM we see two files:

```shell-session
level09@SnowCrash:~$ ls -l
total 12
-rwsr-sr-x 1 flag09 level09 7640 Mar  5  2016 level09
----r--r-- 1 flag09 level09   26 Mar  5  2016 token
```

2. When we execute level09, we get:

```shell-session
level09@SnowCrash:~$ ./level09
You need to provied only one arg.
```

And printing the content of token gives us:

```shell-session
level09@SnowCrash:~$ cat token
f4kmm6p|=pnDBDu{
```

But that string does not allow us to log in as `flag09`.

So this must be encrypted in some way.

3. Feeding the `token` file to the executable:

```shell-session
level09@SnowCrash:~$ ./level09 token
tpmhr
```

This output is weird, the binary seems to reduce the length of the token ? Or do something entirely different ?

4. Understanding what the binary does:

We are visibly not supposed to run `ltrace` or `strace` on the binary, it seems protected against it:

```shell-session
level09@SnowCrash:~$ strace ./level09
execve("./level09", ["./level09"], [/* 18 vars */]) = 0
[...]
write(1, "You should not reverse this\n", 28You should not reverse this
) = 28
exit_group(1)                           = ?
```

```shell-session
level09@SnowCrash:~$ ltrace ./level09
__libc_start_main(0x80487ce, 1, 0xbffffd04, 0x8048aa0, 0x8048b10 <unfinished ...>
ptrace(0, 0, 1, 0, 0xb7e2fe38)                            = -1
puts("You should not reverse this"You should not reverse this
)                       = 28
+++ exited (status 1) +++
```

Are we supposed to deduce what the binary does by experimenting ?

## reverse engineering

So lets proceed with feeding different files to the binary to try to understand what it does:

```shell-session
level09@SnowCrash:~$ echo "test1" > /tmp/test1
level09@SnowCrash:~$ ./level09 /tmp/test1
/uos3ykz|:
```

```shell-session
level09@SnowCrash:~$ echo "test2" > /tmp/test2
level09@SnowCrash:~$ ./level09 /tmp/test2
/uos3ykz|;
```

The binary outputs nearly the same string, so what if we change the content of the `test2` file?

```shell-session
level09@SnowCrash:~$ echo "test3" > /tmp/test2
level09@SnowCrash:~$ ./level09 /tmp/test2
/uos3ykz|;
```

It outputs the same string !

So surely the binary merely takes a string and does not open the file you send into it.

What if we feed it a simple string ?

```shell-session
level09@SnowCrash:~$ ./level09 0000000
0123456
```

That's a very informative output, we can easily deduce what the binary does !
It rotates the value of each character in the string based on its position in the string !

And that's a very easy algorithm to reverse !
what if we launch the algorithm on the contents of the `token` file ?

```shell-session
level09@SnowCrash:~$ ./level09 "f4kmm6p|=pnDBDu{"
f5mpq;vEyxONQ
```

But, after trying, that string does not allow us to log into the VM as `flag09` either...

So maybe we should try and decrypt it by making a script with an algorithm that does the opposite than what the `level09` binary does ?

## collecting information

1. the token must be encrypted, because it is of no use to us for now.
2. the binary takes a string and not a file.
3. the binary rotates the value of each character of that string by the index of said character
4. we must make a script that does that algorithm in reverse to test if the `token` is the result of that algorithm and if so, if that will allow us to log into the VM as `flag09`

## reverse-engineered

We can write a simple C program that does the exact reverse of what that binary does:

```C
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    for (size_t i = 0; argv[1][i] != '\0'; i++)
      dprintf(STDOUT_FILENO, "%c", (argv[1][i] - (char)i));
    dprintf(STDOUT_FILENO, "\n");
    return (EXIT_SUCCESS);
  }
  else
  {
    dprintf(STDERR_FILENO, "USAGE:\n\t./a.out <string to decode>\n");
    return (EXIT_FAILURE);
  }
}
```

Then feed the contents of the `token` file to that program:
```shell-session
λ snowcrash42/level09/Ressources scp -P 4242 scp://level09@192.168.1.44/token encrypted_token
β snowcrash42/level09/Ressources gcc main.c
λ snowcrash42/level09/Ressources ./a.out `cat encrypted_token`
f3iji1ju5yuevaus41q1afiuq
```

## getflag

And then use that hash to log in as `flag09`:

```shell-sesison
flag09@192.168.124.166's password:
Don't forget to launch getflag !
flag09@SnowCrash:~$ getflag
Check flag.Here is your token : s5cAJpM8ev6XHw998pRWG728z
```
