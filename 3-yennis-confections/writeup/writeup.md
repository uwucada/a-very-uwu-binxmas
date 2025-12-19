# yennis confections 

well done team! we robbed yennis confections, got ourselves some sweet cakes and even made off with a couple pies! 

this challenge brought two new concepts we didn't handle in previous challenges, but we overcame and we earned ourselves some tasty dog treats!

this one introduced two new concepts, and one of them wasn't even in the code! 

![free-wallpaper](/3-yennis-confections/wallpaper/yennifur_wallpaper.png)

## overview 

the first thing we may notice is that unlike other times, we don't have a secure password protected vault. 

we don't have much of anything, really. 

if we look at the source code, we can see the print_flag function, but it's never called. 

it is not possible for this app to ever, in any situation, to actually print the flag.

so we're going to have to rewrite it at runtime 😎

![print-flag-occurrence](/3-yennis-confections/writeup/attachments/print-flag-occurrence.png)

## some useful tools 

i have more-or-less entirely avoided using any external tools up to this point, except for some very simple python scripting, because i think it's quite important to be able to do these things by hand, however, at this stage, you may want to get some tools. 

i would recommend stocking up on: 
- gdb (debugger), 
- pwndbg (plugin for gdb to make it better at binex), 
- pwntools (python library for helping build exploits), 
- ROPgadget (cli tool for detecting rop gadgets in binaries)

you're not gonna need 'em all right now, right now we need to learn about return oriented programming (rop)

## rop 

return oriented programming is a method of creatively chaining other simple exploits together, such as overflows, so that we can leverage 'rop gadgets' inside a binary to build in new functionality or rewrite existing functionality, typically using 'ret' statements to jump around the application. 

these gadgets themselves are just little chunks of machine code that often nobody put there on purpose, the compiler just kinda ended up doing that. 

you can kinda think about this similarly to the first stack overflow we did in challenge 1, where we forced the app to ignore an auth check, it's not normally meant to do that, but what we have to do here is a little more complex. 

in this case, print_flag requires an argument, we can't just jump there, it'll fail, what we really need is a formal call to the print_flag function that passes the secret key we need to decrypt the flag. if you look at the source, there's a global variable `secret_key` that holds the xor key - we need to pass its address to print_flag.

so let's write one. 

![print-flag-sig](/3-yennis-confections/writeup/attachments/print-flag-signature.png)

## picking a gadget

so, ideally, the way you'd go about building a rop chain is to run a tool like `ROPgadget`

you can run it like `ROPgadget --binary *binary*` and it'll disassemble the binary and list all the rop gadgets it discovers.

realistically, you'll probably not know which one to go for right off the bat, it's a lot, and you have to know the x64 architecture pretty well to really know what everything does. if this ends up being fun for you, i'd recommend learning more about the architecture, but my goal here is to get you puppygirl wallpapers, so we'll keep it light. 

there are some good starters though, in this case, you can see i hardcoded the pop %rdi in the source code, so that should be a hint that that's the one we're looking for.

![rdi-gadget-ropg](/3-yennis-confections/writeup/attachments/rdi-gadget-ropg.png)
![hardcoded-gadget](/3-yennis-confections/writeup/attachments/hardcoded-gadget.png)

## the pop rdi; ret

so let's break down what this gadget actually does.

you can think of it as two separate instructions rolled into a oneliner like when you're writing powershell but you don't wanna get stuck in that god-awful multiline editor.

the first, `pop %rdi`: this means that we'll `pop` a value off the stack and place it into the `rdi` register.

we haven't really touched on registers before so lets do so here quick:

registers are little blocks of easily accessible memory used to momentarily store things by the CPU. many registers are general-purpose and are just kinda used to store whatever, but some, including `rdi` has a very specific fuction (in Linux x64). in Linux x64, this register holds the first argument to a function. whenever a function is called, if it accepts an argument, it'll pull it from `rdi`. registers are super small, and defintely not meant to store a whole ton of data, so they're usually used for stack data (as opposed to heap data).

so what value are we going to pop off the stack and into `rdi`? the `pop %rdi` instruction takes whatever value is currently at the top of the stack (at the stack pointer) and moves it into the `rdi` register. we'll use our overflow to place the address of the secret key on the stack, so when `pop %rdi` executes, it'll load that address into `rdi` as our function argument.

so, we're clearly going to want to put the address of our secret key on the stack so it gets loaded into `rdi`.

cool.

the second part, `ret`, pops the next value from the stack and jumps to it. so after `pop %rdi` executes and moves the stack pointer forward, `ret` will pop whatever's next (which will be the address of print_flag) and jump there.

so then, if we place both the address of our secret key and the address of our print function on the stack in the right order, the gadget will load the key as an argument, and then jump to the print function, and we'll get our cakes!

## how tf do we do that

that's actually really simple!

the first thing we're going to want is a stack-based buffer overflow, just like we had in challenge 1!

then, all we need to do is overflow with a specific payload, just like we had in challenge 2!

what's the payload? we'll want to use our overflow padding, as we're used to at this stage, then overlfow the return pointer with the address of the rop gadget, so that the function returns to our gadget, BUT then we're going to want to also add another few bytes onto the end of our payload, comprising the address of the secret and the print_flag!

## writing our exploit 

this binary requires 2 strings, the first isn't that helpful, but the juicy bug we want to exploit is actually in the second input. 

this is a good time to talk about another cool tool that'll make our lives easier, `pwntools`. 

this is a python library that makes building exploits a little easier for us. 

we can tell it to load the binary for us, wait for it to get to a certain point, send a string, and then keep going, before ultimately sending our real payload. this also makes it much easier to automate exploitation after things like leaks (foreshadowing)

so we write our exploit.... and it fails. 

![pwntools](/3-yennis-confections/writeup/attachments/pwntools.png)
![failed-exploit](/3-yennis-confections/writeup/attachments/failed-exploit.png)

## introducing PIE

we haven't touched on binary security yet, but this is a pretty good time to do so. in all the previous challenges, we've been able to use the addresses we get from objdump quite easily, because they're hardcoded. if you look back at those, you'll see that we actually explicity have a -no-pie flag in the build script. 

so what does PIE do? well, your operating system is running something called ASLR, which stands for "Address Space Layout Randomization". ASLR already randomizes where the stack and heap are loaded, but PIE (position independent executable) extends this to also randomize the base address of the executable's code and data sections at runtime, so the memory addresses are never identical between runs. 

notice how I say the base address? the offsets between different operations must stay consistent, otherwise the app won't run. when you compile an app with PIE enabled, it's basically telling the compiler to build the app such that it considers offsets between operations rather than using absolute base addresses, allowing the whole thing to be loaded anywhere in memory. 

this is quite a pain in the ass for would-be attackers, since it effectively means unless we get really fn lucky and memory is allocated exactly where we expect it to be, we're not going to get a workable exploit. 

this means we need _another_ vulnerability first. we need something to leak an address of something we can reference back to what we do know, without killing the app. 

from objdump, we can see the offset, but if we have the relative position of something we know, we can figure out the base address, and if we can do that, we can calculate the actual addresses of everything at runtime, bypassing PIE.

luckily, this app has a debug flag, and if we run that, we get an address, neat! 

## writing our exploit (redux)

this is where pwntools again comes in really handy, because we can tell it to catch certain values as they appear in stdout, and we can do stuff with them! 

so, in this case, we catch the address leak value, subtract the main offset, which we can see from objdump, giving us the PIE base address, and use it to calculate the addresses of all of the other goodies we need and BAM! 

![exploit-working](/3-yennis-confections/writeup/attachments/exploit-working.png)


## wrap 

i hope you enjoyed this one! it's quite simple really, but it's got some catches, and i hope it taught you something! 

i hope you'll join me next challenge as we rob all Yin's hidden treasures!
