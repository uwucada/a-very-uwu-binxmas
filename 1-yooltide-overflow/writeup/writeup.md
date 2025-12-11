# yooltide overflow 

this is a classic and very simple example of a stack-based buffer overflow. 

the idea here is to get you acquainted with overflow math and stack logic and to get a sweet new wallpaper of yulia who is wonderful and amazing 

## overview

you get a 'secure' password-protected vault that will ask for a password. if the password is correct, it'll print out the flag for us.

unfortunately, we don't know the password so we're not going to get very far that way, we'll need to be naughty little grinches and steal all of yulia's presents for ourselves. 

![prompt](/1-yooltide-overflow/writeup/attachments/prompt.png)

## the source code 

looking at the code, we can see that we declare a 64-byte buffer and then run `gets(password_buffer)`

`gets` is a bit of a joke function because it's so insecure, and has been removed from the C standard for a while, but essentially what this tells us is that we're going to get _whatever_ data the user writes to the cli and we're going to try stuff it into `password_buffer`, regardless of the size of the string that's provided

let's dig into some computer science so we can find out why that's bad 

![vulnerable_code](/1-yooltide-overflow/writeup/attachments/vulnerable_code.png)

## the stack 

a running process can more or less be divided into 3 parts:

1. data segment - global and static variables, constant data, etc. 
2. the stack - holds control-flow data, which effectively is any of the data required for actual _computation_, this includes addresses, parameters, local variables, etc. it's called the stack because it operates on a LIFO (last-in-first-out) basis where you literally put new data on top of old data, resolve that data, and remove it, like stacking plates when you're washing dishes.
3. the heap - used for runtime memory allocation or where the size of memory required isn't known at compile time, typically holds data like the content of files you're reading from disk. 

all binary exploitation is cool but stack exploitation is big W's because by attacking the stack we can essentially rewrite the app _while it's running_ to do cool stuff it's not meant to do, which we're going to here a little, but will do a lot more of in later challenges where we build simple ROP chains (don't worry about this rn)

when a function is called, that creates what's called a "stack frame", which would effectively be one of the plates on the stack. 

the stack frame has a specific structure to it that is always followed, including what's known as a 'function prologue'

in the function prologue, two main things happen: 

1. the return pointer is added to the stack - this is an 8-byte value that holds the address of where in the app we're supposed to go after this function `returns`
2. the saved base pointer is added to the stack - this is also an 8-byte value that holds the address of the base pointer of the _previous_ stack frame 

if we're looking at a disassembly of a binary, which we'll do later, it'll look something like this: 
```
get_password();
; var char *s @ stack - 0x48
0x004012f5      push    rbp < return pointer 
0x004012f6      mov     rbp, rsp < saved base pointer
```

the gold standard here would be to rewrite the return address of our function so that when our function returns, instead of returning back to where it was going to return initially, we can get it to execute _a different function_. 

![stack_frame_explanation](/1-yooltide-overflow/writeup/attachments/stack_frame_explanation.png)

## overflows 

now, the way that an overflow works is actually very simple. 

if we try to shove, say, 67 bytes into our 64-byte buffer, we're going to have a problem, because 67, i'm told, is a bigger number than 64, so it won't fit. 

where's it going to go, though? well, the stack grows down, which means that new stack allocations are placed under old allocations if you visualize it vertically, so when we overfill something, it spills up, which is to say that those bonus bytes will overflow _upwards_ and overwrite the next adjacent buffer or in simpler terms we'll spill up into last buffer that was allocated on the stack.

therefore, if we stuff 67 bytes into this buffer, we'll fill up this buffer and also write 3 total bytes into the saved base pointer's buffer. 

but we don't want to overwrite the saved base pointer buffer, we want to overwrite the return address, so if we're doing some quick math, the total amount of memory we would need to shove into our buffer is: 
64 (our buffer) + 8 (saved base pointer) + 8 (return address) = 80 bytes!

## exploitation strats

if we're overwriting an address tho, we'd want to overwrite it with a new address that helps us out in some way, so the question becomes, where do we wanna go?

well, taking another look at the source, we see that if we get the password right, it'll print the flag for us, so, why not just skip the middleman and just run that?

![conditional](/1-yooltide-overflow/writeup/attachments/conditional.png)

## finding the address 

to do that, we'll need to find the address of the print_flag function, which we can happily do by dissassembling the object with objdump (or whatever other tool you like, idc): 

![objdump](/1-yooltide-overflow/writeup/attachments/objdump.png)

## exploitation 

great! we've got the address we need and we've got the strat on how to get there. how do we actually pop it?

well, in more complex cases we may want to write some pwndebug scripts in python, which we will do later, but for now, we can make this very simple with a little python oneliner, or if you can count all the way to 80 you can even do it by hand 

i can't count to 80 so i'm going to write python

one note here is that we have to reformat the address into a friendly hex format by splitting every two characters into a byte pair, and then we have to reverse it because we're in little endian (take note of this but don't worry too much about it atm - it means the bytes are stored in reverse order).

![exploit](/1-yooltide-overflow/writeup/attachments/exploit.png)

## finishing it off 

now we just run the exploit, and we have ourselves a secret pass into yulia's yooltide present vault

![exploit_proof](/1-yooltide-overflow/writeup/attachments/exploit_proof.png)

## wrap 

one problem you may run into is that when we pipe commands into each other sometimes outputs get 'buffered', which is to say, the target app actually crashes _before_ the output is printed, that's why we use stdbuf to disable output buffering and print everything immediately as it happens. 

it's pretty neat though, isn't it? because we overwrite the _return_ address of the function, we can see it run through the entire function as if we hadn't broken it before the payload we launch actually detonates on the return!

1. ask for a password
2. check the password
3. FAIL the password check 
4. function _returns_ and runs the print_flag function anyway 
5. crash 

the crash here is normal and expected for this type of bug, so don't panic if you see a segfault, in this case we're going for memory corruption so it's actually a good thing! 

hope you'll join me next time when we exfil heaps of presents from yuma's spooky cauldron 😎

