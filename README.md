# Laboratório de Sistemas Operacionais

Nove programas em C para estudar como aplicações Linux acessam o kernel, criam processos e coordenam threads. Cada exemplo demonstra um conceito e compila em um executável independente.

## Comece aqui

Use Linux ou WSL 2 com um compilador C, GNU Make e os utilitários básicos do sistema. Na raiz do repositório, execute:

```sh
make -j
make test
```

Os executáveis ficam em `build/`. O Makefile usa C17, habilita avisos e passa `-pthread` aos exemplos da seção de threads. Use `make clean` para remover os binários.

## Exemplos

| Tema | Programa | O que observar |
|---|---|---|
| Syscalls | [kernel_pid.c](examples/syscalls/kernel_pid.c) | Compare o PID retornado pelo wrapper específico da libc e por `syscall(SYS_getpid)`. |
| Syscalls | [copy_file.c](examples/syscalls/copy_file.c) | Copie os bytes de um arquivo com `open`, `read`, `write` e `close`. O destino precisa ser novo. |
| Syscalls | [file_seek.c](examples/syscalls/file_seek.c) | Posicione o descritor com `lseek` e leia o byte naquele deslocamento. |
| Processos | [fork_wait.c](examples/processos/fork_wait.c) | Mude uma variável no filho e observe o valor no pai depois de `waitpid`. |
| Processos | [fork_exec.c](examples/processos/fork_exec.c) | Execute outro programa no filho e confira o código de saída recebido pelo pai. |
| Processos | [pipe_ipc.c](examples/processos/pipe_ipc.c) | Passe uma mensagem entre pai e filho com um pipe e observe o fim da leitura. |
| Threads | [pthread_join.c](examples/threads/pthread_join.c) | Entregue argumentos a duas threads e leia os resultados depois de `pthread_join`. |
| Threads | [mutex_counter.c](examples/threads/mutex_counter.c) | Proteja os incrementos de um contador compartilhado com um mutex. |
| Threads | [condvar_queue.c](examples/threads/condvar_queue.c) | Coordene um produtor e um consumidor com um buffer de uma posição e variáveis de condição. |

## Roteiro rápido

```sh
./build/syscalls/kernel_pid

pasta=$(mktemp -d)
printf 'ABC\n' > "$pasta/origem.txt"
./build/syscalls/copy_file "$pasta/origem.txt" "$pasta/copia.txt"
./build/syscalls/file_seek "$pasta/copia.txt" 1
rm -r -- "$pasta"

./build/processos/fork_wait
./build/processos/fork_exec
./build/processos/pipe_ipc

./build/threads/pthread_join
./build/threads/mutex_counter
./build/threads/condvar_queue
```

O deslocamento começa em zero. Por isso, a leitura na posição `1` de `ABC` mostra `0x42`, o valor hexadecimal de `B`.

## Conceitos para acompanhar o código

- **Syscalls.** Programas pedem serviços ao kernel. Aqui, `getpid()` usa o wrapper específico da libc e `syscall(SYS_getpid)` usa o wrapper genérico da mesma biblioteca. A função `open()` também é um wrapper, e em Linux pode aparecer como `openat` no rastreamento. [Manual de `syscall(2)`](https://man7.org/linux/man-pages/man2/syscall.2.html) · [Manual de `open(2)`](https://man7.org/linux/man-pages/man2/open.2.html)
- **Processos.** Depois de `fork()`, pai e filho têm espaços de memória separados. Descritores herdados podem referenciar a mesma descrição de arquivo do kernel, que contém o deslocamento. O pai usa `waitpid()` para obter o estado de saída do filho. [Manual de `fork(2)`](https://man7.org/linux/man-pages/man2/fork.2.html) · [Manual de `waitpid(2)`](https://man7.org/linux/man-pages/man2/waitpid.2.html)
- **Threads.** As threads compartilham a memória do processo. O mutex protege o acesso concorrente ao estado e a variável de condição permite esperar por uma mudança nesse estado. Funções `pthread_*` como `pthread_create` devolvem diretamente o código de erro. [Manual de `pthreads(7)`](https://man7.org/linux/man-pages/man7/pthreads.7.html) · [Manual de `pthread_cond_wait(3p)`](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html)

Com o `strace` instalado, você pode acompanhar as syscalls dos exemplos. A opção `-f` segue filhos e threads.

```sh
strace -e trace=getpid ./build/syscalls/kernel_pid
strace -f -e trace=clone,clone3,fork,vfork,execve,wait4,waitid ./build/processos/fork_exec
strace -f -e trace=clone,clone3,futex ./build/threads/condvar_queue
```

Para praticar, siga o [roteiro de exercícios](docs/roteiro.md).
