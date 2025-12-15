# yuma's present cauldron

similarly to the last challenge, this one is a super simple, classic challenge you'll find in most any CTF

the only real difference here is that in this case, we're playing in the heap, not the stack 

![free-wallpaper](/2-yumas-present-cauldron/wallpaper/yuma.png)

## overview 

again, we get a 'secure' password protected cauldron, where, if the password is correct, it'll dump the flag for us. neat! 

however, just like last time, we unfortunately don't know our password, so we're going to need to make like hansel and gretel and rob this poor lil witch blind 

![prompt](/2-yumas-present-cauldron/writeup/attachments/prompt.png)

## the source code 

this time, we again allocate a buffer for the input from the user, this time 256 bytes, but we don't use `gets` to populate this. instead, we use `fgets`, which we can see takes a few extra arguments. 

most notably, its passed the actual size of the input buffer. 

this means that this operation is length limited, we will not be able to overflow the input buffer here. 

fgets will take at most the first 255 characters of whatever we put in, and then terminate it with a null. 

the actual vulnerability here is in this line: 

`strcpy(data->password, input)`

but to understand why, we're going to need to learn a little more about how memory allocation on the heap works 

## the heap 

last time, we mentioned that there are 3 parts to a running app: the data segment, the stack, and the heap.

the last challenge focused on exploiting a stack based buffer overflow, which allowed us to redirect the control flow to force the `print_flag` function to run when it wasn't supposed to.

since, in normal operation, the heap doesn't *directly* influence control flow, we can't really do the same thing here. 

instead, the heap is where we store data that has to be allocated at runtime, that is, that we maybe don't know the size of at compile time or that's too large or whatever the case may be. 

we can tell heap allocations from stack allocations in source because they need to be called explicitly with functions like `malloc()`.

another cool thing about the stack is that because everything is inherently allocated contiguously, it means you kinda always inherently know *where* everything is. 

this is not the case on the heap as heap allocations are not necessarily contiguous at all, which is why all heap allocation primitives like `malloc()` return pointers, because otherwise you wouldn't know where your new heap buffer is. 

due to the heap's unpredictable allocations, you'll often see that more sophisticated attacks involve something called **heap grooming**, **heap feng shui**, or similar. what these actually mean is that the attacker is forcing allocations into the heap to try fill up space, to ensure that their next attack against that heap actually hits something valuable, and isn't just dead air. 

this isn't the case here though, because there are some cases where heap memory is allocated contiguously, most notably in custom allocators like memory arenas, ring buffers, and for our case **structs**.

in C, fields inside a struct are always allocated contiguously and in the order they're declared, so if there is some important value stored on the heap and you can overflow an adjacent field in that same struct, you're good to go 

## exploitation strat

in the last challenge, we had to do some quick math to figure out how much we needed to overflow by, and we needed to do some digging to find the address we actually wanted to overflow *to*.

this time? much easier. all we need to do is literally overflow our buffer and then append the value we want to overflow into the adjacent buffer. that's all. ezpz.

we can see the sizes we need here: 

```
#define BUFFER_SIZE 64

struct user_data {
  char password[BUFFER_SIZE];
  char auth_status[BUFFER_SIZE];
};
```

and we can see the auth decision here: 

```
const char* authenticated_state = "AUTH_STATUS: authenticated";
...
  if (hash_result == 0) {
    strcpy(data->auth_status, authenticated_state);
  }

```

if the auth status field is that value, we get our flag. 

## exploitation 

so then, all we need to do to write an exploit is:
 
![exploit](/2-yumas-present-cauldron/writeup/attachments/exploit.png)

## finishing off 

now we just run our exploit and we can see the following chain play out: 

1. we get asked for our input 
2. we try enter our password, which overflows the auth state 
3. the password is found to be invalid 
4. since we check the auth state and not just the password, we print the flag 

![exploit-proof](/2-yumas-present-cauldron/writeup/attachments/exploit-proof.png)

## wrap

we managed to get some presents out, but unfortunately we all toads now :c still, a small price to pay as long as you had fun or learned something new! 

join me tomorrow for our next heist stealing some cakes and pies in 

Challenge 3: the Caper at Yennifur's Kennel & Confections!

## notes

i wanna just touch on some comments i've got and some i expect to get but didn't really have space for in the main thing?

**unauthenticated is longer than authenticated, where did those extra characters go?** 
in C, strings are null-terminated, so whenever you're passing around strings, the compiler will silently stick a null byte on the end. since a null byte would never exist in a normal string, this presents an easy way for a function to very quickly tell where a string ends, because it ends on the first null byte.

whenever you're doing string operations like comparisons, it'll stop reading after the null, so technically, if you overflow a string with a smaller value, then fragments of the original value are still there in memory, they're just ignored, because they come _after_ the null byte that tells the computer the string is over.

**what's to stop us decoding the flag from the binary?**
nothing at all. 

it's not super feasible for me to encrypt the flag inside the same binary while giving you source code in a way that's not possible to reverse. 

no matter what i did, the easiest thing you could possibly do would be to add a line or two into the source code to just print the flag immediately and recompile it. 

it's not really something i'm interested in doing, though as these get harder it will get harder to fetch the flag from the source code too. 

up till now, this hasn't exactly been cryptmas either, the runtime key we xor against is just zeroes. it doesn't do anything, it's just there to look like it's doing stuff, but if you run strings on the binary you can see the flag in clear text

![bad-crypto](/2-yumas-present-cauldron/writeup/attachments/bad-crypto.png)
