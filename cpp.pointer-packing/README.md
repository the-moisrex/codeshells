# Pointer Packing

Source: https://www.1024cores.net/home/lock-free-algorithms/tricks/pointer-packing


On modern architectures atomic RMW (read-modify-write) operations are able to operate either on single words or on double words, that is, on 64/128 bits on 32/64 bit architectures at most. Moreover, on some 64 bit architectures (some early AMD64, SPARC) double word atomic operations are not available.

And sometimes it's necessary to fit more data into that limited width. If we need to fit one or several pointers, then we are lucky, because pointers usually contain some unused bits. Let's see what we can do.

Due to alignment requirements several low bits of pointers are always zero. To get to know as to how many low bits are zero, you need to consult documentation on alignment for your memory allocator. However, it's usually at least 3 or 4 bits (that is, 8 or 16 byte alignment). Some allocators provide 64 byte alignment to prevent false-sharing (that's 6 zero bits). Alternatively you can ensure specific alignment with functions like _aligned_malloc()/posix_memalign(). So, this way we can take away 3-6 low bits.

Current Intel64/AMD64 architecture provides 48-bit logical addressing (yes, with respect to memory addressing they are not 64-bit machines, they are 48-bit). Moreover, Windows limits logical addresses to 44 bits, plus 44-th bit is always set for kernel-space addresses, and reset for user-space addresses. So, on Windows we can take away 21 high bits. On Linux situation is worse in this respect, because it provides fair 48-addressing, so we can take away only 16 high bits. However, be careful with that, because logical address space can be extended in future.

So on Windows/Intel64 pointer packing/unpacking may look as:

```c++
size_t const packed_ptr_size = 39;

size_t const lo_bits = 4;

uintptr_t const lo_mask = ((1ull << 4) - 1);

uintptr_t const hi_mask = ~((1ull << 43) - 1);

uintptr_t ptr_pack(void const volatile* p)

{

    uintptr_t v = (uintptr_t)p;

    assert((v & lo_mask) == 0);

    assert((v & hi_mask) == 0);

    v = v >> lo_bits;

    return v;

}

void* ptr_unpack (uintptr_t v)

{

    uintptr_t p = (v << lo_bits) & ~hi_mask;

        return (void*)p;

}
```

That's actual trick used in implementation of Interlocked Singly Linked Lists in Windows. And, as far as I understand, that is the reason behind artificial limiting logical addresses to 44-bits - they need to fit a pointer (39 bits) + node count (16 bits) + ABA counter (9 bits) into 64 bits. If addressing would be fair 48 bits, then it would leave only 5 bits for ABA counter which is indecently small.

Such packing allows one to fit, for example, 3 pointers and an 11-bit counter into 128 bits (3*39 + 11 = 128).

Of course such tricks are quite dangerous, and when you apply it you better carefully check processor/OS versions beforehand. Most likely you need some backoff mechanism (perhaps mutex-based) as well. Alternatively you can consider it vise versa - you have a main portable mutex-based algorithm, and an optimization for some platforms.

However, there is safer and portable way to get "smaller" pointers - we may allocate objects from a fixed-size pool, so that pointers can be replaced with offsets into the pool. If objects are fixed-size as well, then we may consider a pool as an array and pointers as indexes into it. For example, if we reserve 4GB for the pool (no need to actually allocate them, we just need to reserve a continuous region of memory in process's address space), and object size is 128 bytes, "pointers" effectively become only 25 bits - we can fit 5 (!) such pointers into single 128 bit double word and operate on it atomically with CMPXCHG16B.
