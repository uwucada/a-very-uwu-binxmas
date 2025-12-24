# yin's hidden treasures 

congratulations to challenge completers! 

this challenge didn't introduce anything new, really, the challenge was more about understanding how attacker-controlled data propagates through the code and combining the things we'd discussed in previous challenges.

some call this taint analysis but i don't because that is not a good enough reason to use the word taint

let's dig into it!

![free-wallpaper](/4-yins-hidden-treasures/wallpaper/yin.png)

## overview 

instead of an executable binary like we normally get, in this case, we get an .mp3

this isn't much of a challenge at all, to be honest, it was just a little homage to Yin's propensity of hiding things in songs. 

if we run `strings` on the song, we can clearly see that there are references to GLIBC in the output, which tells us that there's probably a binary hidden inside the song 

we can use `binwalk` to extract it (sometimes while i was testing this it failed to i had to use `dd` but like it doesn't matter either way)

![strings-output](/4-yins-hidden-treasures/writeup/attachments/strings-output.png)
![binwalk-output](/4-yins-hidden-treasures/writeup/attachments/binwalk-output.png)

## looking at our binary 

so first things first, we can see that it's an ID3 tag parser.

we give it a path to an mp3 and it'll print out the tags for us 

we can also just run `checksec` on the binary to confirm what security flags the binary has enabled, and we see it's again running PIE, just like last time. 

(side note: i fucked this thing up with emojis bc i know Yin HATES emojis and i want her to bully me)

![app-normal](/4-yins-hidden-treasures/writeup/attachments/app-normal.png)
![checksec-output](/4-yins-hidden-treasures/writeup/attachments/checksec-output.png)

## a lower level look 

in previous challenges, we've looked mainly at the source code and treated the disassembly as secondary. this is fine and i think using both gives you a better understanding of what the code that causes these vulnerabilities actually looks like, but i do think we need to go a bit deeper now, as you rarely have the source code to work with in the real world, so let's use the disassembly as our primary reference point for this one and reference the source more to explain what we're seeing in the disassembly. 

i'm going to be using [Cutter](https://cutter.re/) as my disassembler/decompiiler of choice bc i like it, but you're welcome to use Ghidra, IDA, objdump, notepad.exe copilot or whatever you want 

## the first bug: format string 

maybe you found it by looking at the source, maybe you found it by bonking around and trying to input stinky arguments, maybe you found it by disassembly, either way, you found a format string bug, and this is a good opportunity to marry together what we see in the binary, what we've already learned about binary exploitation, and what we see in the source. 

SO 

you find a format string where the app asks for a file to open, in the source it's this line: 

`printf(filename);`

now, we know that the thing that causes format string bugs are when attacker-controlled data is passed to `printf` without being sanitized via format specificiers, or rather: 

```c
printf(filename); // bad 
printf("%s", filename); //good
``` 

how would we see this in the disassembly? well, the answer is actually in challenge 3! 

remember how our rop chain needed to first leverage a `pop %rdi; ret` gadget to move something into the `rdi` register before we called the `print_flag` function?

we can tell how printf was called from the disassembly in a similar way, by checking how many registers were populated before it was called! 

check this excerpt from our vulnerable binary first versus a safe example!

![vuln-bin-fstring](/4-yins-hidden-treasures/writeup/attachments/vuln-bin-fstring.png)
![safe-bin-fstring](/4-yins-hidden-treasures/writeup/attachments/safe-bin-fstring.png)

notice how in the safe version, we're passing both an `rdi` and an `rsi`, whereas only `rdi` is passed in the unsafe one? this tells us that either we're passing an actual string literal, or we're passing a variable without a format specifier, since doing so would require that we pass another argument to the function, and that argument would go into `rsi`

only one register is populated, so only one argument is given to `printf`, which means that if that argument is attacker controlled, we have a format string!

## what does that give us 

previously, we saw that with PIE enabled, we needed to first leak an address at runtime that we could map to an address in our static disassembly to determine the PIE offset so we could figure out all the other addresses we wanted to hop to.

last time, yenni just gave it to us, but yin is not so generous 

in this case, we'll need to exploit the format string to leak a code address. 

but we haven't really spoken about format string bugs and how they work, the crux is this: 

printf takes a number of arguments, the first, a kind of template string, which may contain string literals or may contain format specifiers, (`%s`, `%d`, etc). 

let's use an example like the below 

```c
printf("%s", string);
```

in this case, `printf` is using a format specifier `%s` which tells printf to print the value of the `string` variable passed via `rsi` _as if it were a string  literal_. 

if we pass `printf` 7 format specifiers, it'll expect that there's data in 5 registers (`rsi`, `rdx`, `rcx`, `r8`, `r9`), and the 6th and 7th specifiers will read from the stack.

this would hopefully include an address that we can use for our offset calculations! 

so let's pop this bad boy open in `gdb`, feed it a bunch of `%p` (hex-formatted pointer values) specifier and see what we can see! 

doing this, we get a lot of garbage, like a lot. 

this is why we do this in a debugger like `gdb`, because it allows us to suspend execution and query the memory addresses at runtime.

it's still a lot of noise to sift through though, but here's a convenient and highly flawed little rule of thumb for telling types of addresses apart at a glance:

- code addresses usually start with 0x55 or somewhere close to that 
- stack addresses usually start with 0x7fff
- libc addresses also usually start with 0x7fff but will typically look a liiiiittle bit more random

always leak multiple times across multiple app runs if possible to ensure that your assumptions about the memory layout are correct

since we're after beating PIE, we're going to want to leak a code address, and luckily, we do get a few in the leak, namely, we get 0x555555555a17, which gdb tells us is main() , we can also confirm this by looking at the disassembly and checking main's lower bits. 

![fstring-result.png](/4-yins-hidden-treasures/writeup/attachments/fstring-result.png)

## what now 

okay, so now we have our first vulnerability, a format string into an address leak... what do we do with it?

well, we're going to need to find calls to primitives we know can be unsafe, and then check if their actual implementations are vulnerable, so, let's go through our disassembly and see if we can find something spicy we can exploit.

if we take a look at our `process_extended_metadata` function, we actually strike gold! there's an obviously unsafe `memcpy` right there, where we take a maximum of 256 bytes from a pointer and write it to a 64-byte buffer on our stack, which, as we now know, would be more than enough to cause an overflow. 

cool find, but in order to exploit it, we'd need to be able to write arbitrary data to whatever this pointer is referencing. 

if we can do that, we can hijack the control flow, and we can get our flag... wherever that is.

![unchecked-memcpy-disas](/4-yins-hidden-treasures/writeup/attachments/unchecked-memcpy-disas.png)
![unchecked-memcpy-src](/4-yins-hidden-treasures/writeup/attachments/unchecked-memcpy-src.png)

## figuring out the metadata 

now's where things get a little difficult, at least, to do while looking only at the binary and ignoring the source, since tracing these kinds of allocations in pure disassembly is a pain in the ass. but luckily, that's what we have decompilation for! 

decompilation is a process that attempts to generate an approximation of source code based on a compiled binary, which Ghidra, IDA, Cutter, etc can all give you, it just makes it a little easier to read than raw disassembly. 

one word of warning for if you are using Cutter is remember to swap the decompiler to Ghidra's decompiler instead of the default `jsdec` bc that shit is unreadable.

anyway! if we look at the decompilation, we can see that the `ptr` fed to `process_extended_metadata` is a return value from a function called `allocate_tag_buffers`, cool. 

![main-ptr-declaration](/4-yins-hidden-treasures/writeup/attachments/main-ptr-declaration.png)

## checking the memory structure 

if we now descend into the `allocate_tag_buffers` function, we can see an allocation for 320 bytes (0x140) and some weird stuff happening in memory allocations, specifically, we see a single buffer allocated as `iVar2` (320 bytes), and then:

```c
iVar2 = malloc(0x140);
...
*piVar1 = iVar2;
piVar1[1] = iVar2 + 0x40

```

what happens in this code is that we're effectively allocating a block of memory, and then breaking it up into smaller chunks - this may seem a little weird if you're new to low-level code but it's very normal for developers to do. `malloc()` operations are slow and expensive, so often developers will use "custom allocators" for increased performance, where they allocate a single big block of memory and then carve it into smaller pieces after the fact, including allocators like `memory arenas` and `ring buffers`

this is awesome for performance and writing custom allocators is one of those things that makes you fall in love with lower-level code, however, it also means that these piVar blocks are definitionally contiguous, and means that you definitely definitely definitely have to make sure that writes to those buffers never write into adjacent buffers (foreshadowing)

we're allocating a single 320 byte buffer, carving out the first 64 bytes for one thing, and using the remainder of the bytes for another thing, but since we haven't read the source code (you haven't read the source code have u?), we don't know what it's for yet.

still... we do love to see some contiguous memory. 

![allocate-tag-buffers-disas](/4-yins-hidden-treasures/writeup/attachments/allocate-tag-buffers-disas.png)

## finding a write 

so now we see where we've got some memory that could potentially be leveraged for an attack, but we don't yet have an actual write into that memory that would allow us to... leverage it. 

going back to the decompilation of main, though, we can see that the pointer is passed to `parse_id3v2_tags()`, which sounds like the type of place that's likely to have a write primitive for us

that function itself calls out to `parse_id3v2_frame` multiple times, with arguments like "TIT2", "TPE1", "TALB", etc.

these are ID3 tag headers, if the function name didn't give it to you already. 

what we see in that function, we do in fact have a vulnerable write primitive. 

specifically, for TIT2 headers (the title), the code calculates `max_len = content_size / 2` to handle UTF-16 encoding (2 bytes per character), while other tags use `buf_size - 1`. 

the bug is that `content_size / 2` isn't checked against the actual buffer size, so a large enough `content_size` causes `content_size / 2` to exceed the 64-byte title buffer, overflowing into the metadata buffer.

```c
iVar1 = strcmp(arg3, "TIT2");
if (iVar1 == 0) {
    var_30h = (unsigned long long)((uint32_t)var_ch >> 1);
} else {
    var_30h = arg5 - 1;
}
```
if we look back on how the memory structure is created, we'll see that this is actually the same buffer that we 'split' earlier, which means we can overwrite the other part of this buffer!

![unsafe-buffer-check](/4-yins-hidden-treasures/writeup/attachments/unsafe-buffer-check.png)

## getting the building blocks in place

okay, so now we know we have a heap buffer overflow, which we can use to build a stack buffer overflow, allowing us to rewrite the control flow to get us our flag 

we can also see that the `print_flag()` function this time requires us to supply it TWO arguments before we can actually print our flag, luckily, thanks to our experience with yennifur's confections, we know all about registers and how arguments are passed to functions, so we know we need to somehow get these arguments via some pop gadgets. 

fire up ol' ROPgadget and let's see what we can get: 

![pop-gadgets](/4-yins-hidden-treasures/writeup/attachments/ropgadgets.png)

then we need to find the keys themselves so we can actually load them onto our registers 

![keys](/4-yins-hidden-treasures/writeup/attachments/keys.png)

## summarizing what we have 

just to quickly summarize what we have and how we're going to use it: 

### vulnerabilities

1. a format string allowing us to leak a code address
2. a heap based overflow due to unsafe parsing of ID3 title tags 
3. an unsafe `memcpy` that will read from a heap location and allow us to execute a stack based buffer overflow to redirect control flow 

### gadgets and code bits 

1. offset of main 
2. offsets of the rop gadgets
3. offsets of the keys 
4. offset of the print_flag function 

## putting it all together 

our exploit now will need to:

1. run the app and leak a code address, in our case, main()
2. calculate the PIE base 
3. use the PIE base with all of our other offsets to get the runtime addresses of all the gadgets and things we want. 
4. pack a rop chain into the title field of an MP3 file 
5. this will cause our rop chain to overflow the heap buffer into the `extended metadata` structure 
6. the content in this structure now needs to overflow the unsafe `memcpy` in the processing function
7. this will allow us to overwrite this frame's return pointer and execute a rop chain, as we did in previous challenges 
8. the rop chain will need to load the keys into the `rdi` and `rsi` registers and then call `print_flag`

... ezpz 

## exploiting it! 

we write up our exploit, and it doesn't work 

![failed-exploit](/4-yins-hidden-treasures/writeup/attachments/failed-exploit.png)

## troubleshooting 

spending some time scratching your head, you'll realise the rop chain MUST have worked because we can see the functions executing, but why's the flag busted?

it's because when i wrote the challenge i thought it would be funny to swap key 1 and key 2 around, and then i forgot about while playing through the challenge for the writeup

trolled maself. 

## exploiting it! (for real this time)

worked this time :3 

![workin-sploit](/4-yins-hidden-treasures/writeup/attachments/workin_sploit.png)
