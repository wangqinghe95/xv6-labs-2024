# [Assignment Lab: system call](https://pdos.csail.mit.edu/6.828/2024/labs/syscall.html)

## [Using gdb](./anser-syscall.txt)

## System call tracing

`在这个作业中，你需要添加一个系统调用的追踪功能，这将帮助你在后面的实验中调试。你需要创建一个新的 trace 系统调用用于控制追踪。它需要传入一个整数 “mask” 的参数，这个参数的位指定要跟踪的系统调用。举个例子，要跟踪 fork 这个系统调用，程序会调用 trace(1 << SYS_fork), 这个 SYS_fork 是一个定义在 kernel/syscall.h 的系统调用数字。你需要去修改 xv6 内核，在一个系统调用的数字在掩码中被设置的时候该系统调用返回的时候打印一行日志。这行日志应该包含进程 id，系统调用的名称，和返回值。你不需要打印系统调用的参数。trace 系统调用应该能够追踪调用它的进程和它派生出来的任何子进程，但是不能影响其他进程。`

我们提供一个 trace 的用户进程,运行另一个启用追踪的程. 完成后,你应该能够看到如下类似的输出:

```
$ trace 32 grep hello README
3: syscall read -> 1023
3: syscall read -> 966
3: syscall read -> 70
3: syscall read -> 0
$
$ trace 2147483647 grep hello README
4: syscall trace -> 0
4: syscall exec -> 3
4: syscall open -> 3
4: syscall read -> 1023
4: syscall read -> 966
4: syscall read -> 70
4: syscall read -> 0
4: syscall close -> 0
$
$ grep hello README
$
$ trace 2 usertests forkforkfork
usertests starting
test forkforkfork: 407: syscall fork -> 408
408: syscall fork -> 409
409: syscall fork -> 410
410: syscall fork -> 411
409: syscall fork -> 412
410: syscall fork -> 413
409: syscall fork -> 414
411: syscall fork -> 415
...
$
```

上述的第一个示例中, trace 调用 grep 跟踪系统调用 read.  32 是 (1 << SYS_read). 第二个例子中，trace 运行 grep 追踪虽有系统调用。2147483647 有 31 位都被设置。第三个例子中，程序没有被追踪，所以没有追踪输出被打印。在第四个例子中，usertests 中 forkforkfork 测试的子进程的 fork 系统调用被追踪。如果你的程序输出如同上述一样，那么你的解决方法就是正确的。(进程id可能会不同)

一些提示：
1. 添加 `$U/_trace` 到 Makefile 中的 UPROGS
2. 执行 `make qemu`，你会发现编译器不会编译 `user/trace.c`, 因为 trace 系统调用的用户空间还不存在：为 trace 在 `user/usys.pl` 添加一个函数声明，和在 `kernel/syscall.h` 添加一个系统调用数字。 Makefile 引用脚本 `user/usys.pl` 生成 `user/usys.S`, 实际上系统调用 stubs, 它使用 RISC-V ecall 指令转换到内核中。一旦你解决了编译问题，执行 `trace 32 grep hello README`, 这还是会失败因为你还没有实现内核的系统调用
3. 在 `kernel/sysproc.c` 添加一个 `sys_trace()` 函数，在 `proc` 结构体中的一个新变量记录参数的方式实现一个新的系统调用。在用户空间检索系统调用参数的功能位于 kernel/syscall.c ，并且你可以参考他们的使用样例， `kernel/sysproc.c` 。你的新 sys_trace 系统调用数组位于 `kernel/syscall.c`
4. 修改 fork() 从父进程复制跟踪掩码到子进程中
5. 修改 kernel/syscall.c 的 syscall() 函数打印跟踪输出。需要添加一个系统调用的数组到索引中

![Solution](https://zhuanlan.zhihu.com/p/668632093)

## Attack xv6

xv6 内核隔离了用户进程之间以及内核进程和用户进程环境。在上述的作业中，一个应用不能直接调用内核函数或者另一个用户程序。相反只能通过系统调用交互。然而如果系统调用的内部有一个 bug 的话，一个攻击者就能利用这个 bug 打破隔离界限。为了了解 bug 是如何被利用，我们引入了一个 bug 到 xv6 中去，你的目标是利用这个 bug 来欺骗 xv6，进入进入另一个进程。

这个 bug 是，在编译时，忽略了 kernel/vm.c 的 272 行调用 memset(mem, 0, sz) 清除新分配的页面。类似地，当编译 kernel/kalloc.c 时，有两行使用 memset 清空页面的动作被忽略。省略这三行代码（均被 ifndef LAB_SYSCALL）的影响是新分配的内存均保留了之前使用的内容。

```
user/user.h 写入一个 8 字节的秘密到它的内存中，然后释放内存后继续退出。你的目标就是添加几行代码到 uer/attack.c 去找到 secret.c 之前执行时写入到内存的秘密，并且写 8 字节的秘密到文件描述符 2 中。如果 attacktest 打印 "Ok: secret is ebb.ebb" 那么你将获得了系统的信任。注意，每个 attacktest 的秘密都不一样。

你可以修改 user/attack.c，但是你不能做其他的修改，你不能修改 cv6 内核源码，secret.c，attacktest.c 等等
```

一些提示：
+ 在 xv6 shell 中执行 attacktest，它的输出格式如下：` FAIL: no/incorrect secret`
    注意尽管这三行被删，但是 xv6 依旧运行的很好：它从 shell 运行 attacktest。实际上如果你运行 usertest，大部分的示例都能通过。
+ 阅读 user/attacktest.c。它会随机的产生 8 字节的字符串，通过 secrete 程序，将字符串写入到它的内存中。secret 退出以后，attacktest 生成攻击并等待 attack 将秘密字符串写入到文件描述符 2 中
+ 阅读 user/secret.c 并且思路你如何欺骗 xv6 泄露与攻击 attack.c 相关的秘密。
+ 在 xv6 运行 attacktest 测试你的漏洞利用：
    ```
        user/secret.c 会拷贝秘密字节内存中，该内存位于一页开始的 32 字节。将 32 修改为 0，你应该能看到你的攻击不在生效，这是为什么？
    ```

一些小 bug 不会直接影响系统正常运行，但是能够被利用破坏系统的安全性使得系统内核程序变得危险。 xv6 有一些这些都 bug，尽管我们试图尽量在避免它们。那些 比 xv6 有更多代码的内核也有这样的 bug。

## Prerequisition

好的，我现在需要理解用户的问题：“三级页面结构是不是意味着每个进程在创建的过程中，都需要为三级页面的每一页都分配一个物理页”。用户可能是在学习操作系统或xv6相关的课程，对分页机制有疑问。之前已经讨论了xv6的三级页目录，现在用户想知道进程创建时是否需要为每一级页表都分配物理页。

首先，我需要回忆三级页表的结构。三级页表包括页目录（L2）、页中间目录（L1）和页表（L0）。每个进程的虚拟地址通过这三个层级转换到物理地址。用户的问题核心在于是否每一级的所有页表都需要预先分配物理页，或者是否按需分配。

根据之前的知识，操作系统通常不会一次性分配所有页表，而是按需分配。例如，当进程访问某个虚拟地址时，如果对应的页表不存在，会触发缺页中断，操作系统再分配物理页并填充页表项。这样可以节省内存，特别是当进程的地址空间稀疏时。

但用户可能担心进程创建时是否会强制分配所有三级的页表。比如，在xv6中，当fork一个进程时，是否需要为L2、L1、L0都分配页，还是只有部分会被分配？需要回顾xv6的代码实现，特别是进程创建时内存分配的部分。

在xv6的代码中，当创建进程时，会复制父进程的页表。这个过程可能涉及到遍历父进程的页表，并为每个有效的页表项分配新的物理页。但具体到三级页表，是否每一级都需要分配呢？例如，如果父进程的某个L2条目指向一个L1页表，子进程是否需要复制这个L1页表，并分配新的物理页，还是共享？

此外，需要区分页表本身（存储页表项的物理页）和实际的数据/代码页。页表结构本身需要物理页来存储，而进程的数据页也需要物理页。用户的问题可能混淆了页表本身的物理页和进程数据页的物理页。

可能的误解是，三级页表结构意味着每个进程必须有三个固定的物理页分别用于L2、L1、L0。但实际上，L2的页目录可能是一个物理页，但L1和L0的页表可能只在需要时才分配。例如，如果进程的虚拟地址只使用了一小部分，那么可能只有对应的L1和L0页表被分配，而不是全部。

需要进一步解释页表的按需分配机制，以及进程创建时如何处理页表的复制（如写时复制技术）。可能xv6在fork时并不立即复制所有页表，而是延迟到写入时，从而减少初始分配的开销。

总结用户的困惑点：是否三级页表的每一级都需要在进程创建时分配物理页。答案是否定的，操作系统按需分配，只有在需要时才分配相应的页表物理页。进程创建时可能只分配顶级页目录，其他级别的页表在后续访问时逐步分配。




在操作系统的分页机制中，**三级页表结构并不意味着进程创建时必须为每一级页表预先分配所有物理页**。实际上，页表的物理页是按需分配的，只有在进程实际使用到某些虚拟地址时，才会逐级分配所需的页表物理页。以下是详细解释：

---

### **1. 三级页表的基本结构**
以 RISC-V Sv39 分页为例，三级页表包括：
1. **页目录（L2）**：顶级页表，每个进程有一个页目录。
2. **页中间目录（L1）**：第二级页表，由页目录项指向。
3. **页表（L0）**：第三级页表，由页中间目录项指向。

每个层级的页表本身需要占用一个物理页（4KB），用于存储页表项（PTE）。但操作系统不会一次性分配所有层级的物理页，而是根据虚拟地址的使用情况**按需分配**。

---

### **2. 进程创建时的页表分配**
当进程创建时（例如通过 `fork` 或 `exec`）：
1. **顶级页目录（L2）必须分配**：  
   • 进程的页表基址寄存器（如 RISC-V 的 `satp`）需要指向一个有效的页目录，因此 **L2 页目录的物理页必须预先分配**。
2. **L1 和 L0 页表按需分配**：  
   • 如果进程的虚拟地址空间尚未被使用（例如刚创建时），**L1 和 L0 页表的物理页不会被分配**。
   • 只有当进程访问某个虚拟地址时，若对应的页表不存在，才会触发缺页异常，由操作系统动态分配物理页并填充页表项。

---

### **3. 示例：进程首次访问内存**
假设进程访问虚拟地址 `0x1000`，三级页表转换过程如下：
1. **检查 L2 页目录**：  
   • 若 L2 页目录中对应的条目（由 VPN[2] 索引）无效（PTE_V=0），触发缺页异常。
   • 操作系统分配一个物理页作为 L1 页中间目录，并更新 L2 页目录的条目。
2. **检查 L1 页中间目录**：  
   • 若 L1 页中间目录中对应的条目（由 VPN[1] 索引）无效，再次触发缺页异常。
   • 操作系统分配一个物理页作为 L0 页表，并更新 L1 条目。
3. **检查 L0 页表**：  
   • 若 L0 页表中对应的条目（由 VPN[0] 索引）无效，触发缺页异常。
   • 操作系统分配一个物理页存储实际数据，并更新 L0 条目。

通过这种方式，**页表的物理页仅在需要时分配**，避免了为未使用的虚拟地址空间浪费内存。

---

### **4. 进程复制时的优化（如 `fork`）**
在 `fork` 创建子进程时，xv6 使用 **写时复制（Copy-on-Write, COW）** 技术：
1. **共享父进程的页表**：  
   • 子进程的 L2 页目录初始时与父进程相同，所有 L1/L0 页表的物理页暂时共享。
2. **按需分配新页**：  
   • 当父或子进程尝试写入共享页时，触发缺页异常，操作系统分配新的物理页并复制数据。
   • 页表本身（L2/L1/L0）也可能按需复制，但仅在写入时发生。

这种机制进一步减少了进程创建时的物理页分配开销。

---

### **5. 为何不需要预先分配所有页表？**
1. **稀疏地址空间**：  
   进程的虚拟地址空间通常是稀疏的（例如仅使用少量代码和数据），强制分配所有页表会导致大量内存浪费。
2. **性能优化**：  
   按需分配减少了进程启动时的延迟，避免了不必要的内存操作。
3. **物理内存限制**：  
   系统物理内存有限，按需分配是必要的资源管理策略。

---

### **6. 对比：静态分配 vs. 按需分配**
| **场景**               | 静态分配（所有三级页表）           | 按需分配（当前实现）          |
|-------------------------|----------------------------------|-----------------------------|
| **内存占用**            | 极大（可能数百KB甚至MB级）        | 极小（仅分配实际需要的页表）  |
| **进程启动速度**        | 慢（需分配大量物理页）            | 快（仅分配顶级页目录）        |
| **适用场景**            | 仅理论存在，无实际系统使用         | 所有现代操作系统（如Linux）   |

---

### **7. 总结**
• **三级页表结构不要求预先分配所有层级的物理页**，进程创建时只需分配顶级页目录（L2）。
• **L1 和 L0 页表的物理页按需分配**，由缺页异常机制动态管理。
• 这种设计在内存效率、启动速度和灵活性之间取得了平衡，是操作系统的核心优化之一。

通过这种方式，操作系统能够高效支持大量进程的并发运行，同时合理利用物理内存资源。

### Solution

实验目的是为了寻找到能被复用的物理页。内核的物理内存管理方式是通过栈式链表管理的（kernel/kalloc.c 中），secret.c 通过函数 `void *kalloc(void)` 分配物理页，该函数从栈顶取内存页。函数 `void kfree(void *pa)` 将对应的内存页放回栈顶。attack 就是利用这两个函数来实现物理内存服用的目的。

从 attacktest.c 开始分析：

创建一个子进程：

```
int main(int argc, char *argv[])
{
    ... 
    if((pid = fork()) < 0) {
        printf("fork failed\n");
        exit(1);   
    }
    ...
    if(pid == 0) {
        char *newargv[] = { "secret", secret, 0 };
        exec(newargv[0], newargv);
        printf("exec %s failed\n", newargv[0]);
        exit(1);
    }
}

```

#### fork() 过程

在创建子进程的函数中：

```
// kernel/proc.c
int fork(void);
{
    ...
    // np 线程一共分配了 4 页物理页
    if((np = allocproc()) == 0){
        return -1;
    }
    ...

}

static struct proc* allocproc(void)
{
    ...
    struct proc *p;
    // 为 p->trapframe 分配一页物理页
    if((p->trapframe = (struct trapframe *)kalloc()) == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
    }
    ...
    // 一共分配了 3 页
    p->pagetable = proc_pagetable(p);
    if(p->pagetable == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
    }
}

pagetable_t proc_pagetable(struct proc *p)
{
    pagetable_t pagetable;
    pagetable = uvmcreate();            // 分配了一页物理内存
    ...
    // 分配了一页
    if(mappages(pagetable, TRAMPOLINE, PGSIZE,
                (uint64)trampoline, PTE_R | PTE_X) < 0){
        uvmfree(pagetable, 0);
        return 0;
    }

    // 分配了一页
    if(mappages(pagetable, TRAPFRAME, PGSIZE,
                (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
        uvmunmap(pagetable, TRAMPOLINE, 1, 0);
        uvmfree(pagetable, 0);
        return 0;
    }
}

// kernel/vm.c
pagetable_t uvmcreate()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t) kalloc();
  if(pagetable == 0)
    return 0;
  memset(pagetable, 0, PGSIZE);
  return pagetable;
}

// kernel/vm.c
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
    /**
     * for 循环的退出条件是 a == last，size 传入就是 4096，而 PGSIZE 也是 4096
     * 所以这个 for 循环只会循环一次，即只执行一次 walk，也就是一次 walk() 函数
     * walk 里的 for 循环会循环 2 次，但是为什么只会分配 1 页物理页？因为 if 判断语句的跳转？
     **/
    ...
    a = va;
    last = va + size - PGSIZE;
    for(;;){
        if((pte = walk(pagetable, a, 1)) == 0)
        return -1;
        if(*pte & PTE_V)
        panic("mappages: remap");
        *pte = PA2PTE(pa) | perm | PTE_V;
        if(a == last)
        break;
        a += PGSIZE;
        pa += PGSIZE;
    }
}

// 逐级检查页表条目（PTE），若不存在则分配新页（alloc = 1）
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc)
{
    ...
    if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
    ...
}
```

下面再来看子进程在创建的过程中，申请了多少页的物理内存：

还是在 fork() 函数中

```
// kernel/proc.c
int fork(void);
{
    ...
    // 从父进程获取 4 页物理页，自己再申请 2 页物理，后两页也是用来做二级三级页表的
    if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
        freeproc(np);
        release(&np->lock);
        return -1;
    }
    ...

}

// kernel/vm.c

int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
    ...
    // 
    /**
     * sz == PGSIZE；该 for 循环只会走一次
     */
    for(i = 0; i < sz; i += szinc){
        szinc = PGSIZE;
        szinc = PGSIZE;
        /*
         * walk(old, i, 0) 第三个参数是 0，意思是当中间页面不存在的话，函数不会申请新的物理页
         *      该函数本质是从父进程复制他的页表，也就是获取的了 4 页物理页
         *
        */
        if((pte = walk(old, i, 0)) == 0)
            panic("uvmcopy: pte should exist");
        if((*pte & PTE_V) == 0)
            panic("uvmcopy: page not present");
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);
        /*
         * 此处获取一页物理页
         */
        if((mem = kalloc()) == 0)
            goto err;
        memmove(mem, (char*)pa, PGSIZE);

        /*
         * 此处再获取一页物理页
         */
        if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0){
            kfree(mem);
            goto err;
        }
    }
    ...
}

```

所以根据分析，整个 fork() 过程，一共会申请 6 + 4 = 10 页物理页。

#### exec() 过程

exec() 就是通过替换页表的方式来执行一个新的程序。

```
int exec(char *path, char **argv)
{
    ...
    for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
        // 读取要执行程序的 elf 文件
        if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
            goto bad;
        ...
        uint64 sz1;
        // 分配新的物理内存
        if((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, flags2perm(ph.flags))) == 0)
            goto bad;
        // 将段加载到内存中
        if(loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0)
            goto bad;
    }
    ...
}
```

查看一下 user/_secret 的 elf 文件
```
readelf -f user/_secret

Elf file type is EXEC (Executable file)
Entry point 0x5a
There are 4 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  RISCV_ATTRIBUT 0x0000000000007295 0x0000000000000000 0x0000000000000000
                 0x0000000000000033 0x0000000000000000  R      0x1
  LOAD           0x0000000000001000 0x0000000000000000 0x0000000000000000
                 0x00000000000008d1 0x00000000000008d1  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000001000 0x0000000000001000
                 0x0000000000000000 0x0000000000000020  RW     0x1000
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
```

exec() 遍历 elf 文件的所有 program header，将所有 Type = LOAD 的段加载到内存中。方法就是 uvmalloc() 分配物理内存，loadseg() 将段加载到内存中。

这两段会加载到虚拟内存的第 0 页和第 1 页，因为是用户进程，所以该虚拟页属于低地址的用户内存，需要两页的页表和两页虚拟页存放这两个段。

下面是 exec 为用户栈分配内存：

```
  // Allocate some pages at the next page boundary.
  // Make the first inaccessible as a stack guard.
  // Use the rest as the user stack.
  sz = PGROUNDUP(sz);
  uint64 sz1;
  /*
   * USERSTACK 为 1，即固定用户占大小为 1 页，+1 是 stack guard，用户堆栈溢出的处理
   */
  if((sz1 = uvmalloc(pagetable, sz, sz + (USERSTACK+1)*PGSIZE, PTE_W)) == 0)
    goto bad;
```

栈是紧挨着 data 段和 text 段之后分配的，他们同属一个三级页表，不需要额外分配页表，因此栈一共分配了两页。

最后，调用了页表释放函数，释放了旧页表和旧用户内存

```
proc_freepagetable(oldpagetable, oldsz);

void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}
```

uvmunmap() 释放 pte 映射，不会释放物理页，trampoline 是整个操作系统共享，不需要释放， trapframe 是用户态和内核态转换时用到的存储区域，也不会释放。
uvmfree() 释放旧页表占用的内存（5 页）和用户内存（4 页）

综上 exec 一共分配了 3+4+2=9 页，然后又释放了九页。

#### secret.c

```
  char *end = sbrk(PGSIZE*32);
  end = end + 9 * PGSIZE;
  strcpy(end, "my very very very secret pw is:   ");
  strcpy(end+32, argv[1]);
```

secret 使用 sbrk 分配了 32 页内存，并在第 10 页写入数据。

在 attacktest.c 的子进程中，经过 fork,exec，sbrk 之后，此时运行的 secret 一共占用了 10+32 = 42 页

#### wait secret

attacktest.c 调用 wait(0)，获取到 secret 的 proc，调用 freeproc 释放其内存

在 freeproc 的过程中，关注一下内存释放的顺序

```
// kernel/proc.c

static void freeproc(struct proc *p)
{
  // 先释放一页 trapframe
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;

  // 释放页表的内存
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// 从低到高释放用户内存，根据之前内存分配的分析，释放顺序为  data+text，用户栈+page guard，32 页堆内存（每页从低地址到高地址依次释放/入栈），共 36 页
// 最后释放页表，共五页
void uvmfree(pagetable_t pagetable, uint64 sz)
{
  if(sz > 0)
    uvmunmap(pagetable, 0, PGROUNDUP(sz)/PGSIZE, 1);
  freewalk(pagetable);
}
```

此时，我们可以得到 kmem 维护的空闲链表栈，从栈顶开始的页依次是：5 页页表，第 32 页堆内存，第 31 页堆内存 ... 第 10 页堆内存（密码所在页），第一页内存，page guard

#### attack.c

attacktest 运行 attack 进程和 运行 secret 进程是一样的，都是通过 fork+exec。

attack 执行之前，fork 分配 10 页，exec 分配 9 页但是也会释放 9 页，其中 fork 分配的 10 页，有 5 页页表，第 32 ~ 28 页堆内存， exec 分配的 9 页是 第 27~19 页，随后又释放了 9 页回栈顶。
此时栈顶的页的顺序为：9 页，第 18 页堆内存 ... 第 10 页内存。

所以我们只需要在 attack.c  中分配 17 页，而密码就在最后一页内存中。

## Optional challenge exercise
// todo