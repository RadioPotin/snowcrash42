# level12

## Hint

We can see that there is a perl file named level12.pl. (Hide your joy). Let's deep down what's inside shall we ?

First of all there is a `setuid` bit. So we know for sure, like the level04. We will usurp once again his identity.

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

First of all it's something communicating whith localhost on port 4646. Like on level04 with a CGI.

Then we don't have one, but two subroutines. `t` and `n`. From the function call at the bottom we can see that they're nested subroutines. Great.

Let's continue ...

### subroutine `n`
`
This subroutine look's like it's printing two string.
- Either `".."` or `"."`
- It checks the first argument `$_[0]` and if it equal to `1` return `".."` or `"."` otherwise.

So basically it's just checking the return of something.

### subroutine `t`

This one is harder.

We got two arguments : `$_[0]` and `$_[1]`.

They are assigned to a local variable:

`$_[0]` -> `$xx`
`$_[0]` -> `$nn`

Thus the local variable `$xx` is translated with `tr` operator to a capitalized version of the variable.

And it's substitued with the `s` operator and will remove any whitespaces from the string.

For exemple thoses two lines transform a string like:

"This is a String !" to "THISISASTRING!"

We know that we transform the first arguments to a capitalized string without spaces.

The third variable that we can see there is `@outpout`. It's a array assigned by

`egrep "^$xx" /tmp/xd 2>&1`

The egrep call will output the matching lines whoses begin with the transformed string `$xx` in a file located in /tmp named xd, who's redirect stderr to stdout.

Hmmm.

Yes me neither.

Let's see the end of it. (Or not ?)


We now have two variables, one from the arguments who's capitalzed and whitespaced. An other one from the second arguments. And an array assigned from a matchings pattern in a wired file from the first arguments.

Then we have a loop.

```perl
  foreach $line (@output) {
      ($f, $s) = split(/:/, $line);
      if($s =~ $nn) {
          return 1;
      }
  }
```

The loop is looping (Whoohoo) against the array `@output`. For each line we have `$line` assigned.
From this variable it seem's that we split the $line with the character : and store this split in two scalar $f and $s. Like a Key:Value pair.


And if the "Value" from the split is matching to the second argument of the subroutine we return true.


...TobeContinued




Therefore this magificiant perl script. Is printing two string and if a match occurs from a file named /tmp/xd.

Whattodonow.jpg



# Draft

level12@SnowCrash:~$ getfacl level12.pl
# file: level12.pl
# owner: flag12
# group: level12
# flags: ss-
user::rwx
group::r-x
group:flag12:rwx		#effective:r-x
mask::r-x
other::r-x

https://wiki.archlinux.org/title/Access_Control_Lists


https://askubuntu.com/questions/257896/what-is-meant-by-mask-and-effective-in-the-output-from-getfacl

Different permission that shown ?


