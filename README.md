# Laboratório de Sistemas Operacionais

Nove exemplos de sistemas operacionais, cada um com versões em C, Python e Java. O laboratório mostra como programas Linux acessam arquivos, criam processos e coordenam threads, e como as APIs mudam de uma linguagem para outra.

## Comece aqui

Use Linux ou WSL 2 com GNU Make e os utilitários básicos do sistema. Para a versão em C, instale um compilador com suporte a C17. Para as outras versões, use Python 3.10 ou superior e JDK 17 ou superior.

Na raiz do repositório, escolha quais exemplos quer verificar.

```sh
make test          # Compila e testa C
make test-python   # Executa e testa Python
make test-java     # Compila e testa Java
make test-all      # Verifica as três linguagens
```

Use `make` para compilar C e `make java` para compilar Java. Os executáveis C ficam em `build/` e as classes Java ficam em `build/java/`. O Makefile passa `-pthread` aos exemplos de threads em C. Use `make clean` para remover os arquivos compilados.

## Exemplos

| Tema | O que observar | C | Python | Java |
|---|---|---|---|---|
| Syscalls | Compare duas formas de obter o PID. | [kernel_pid.c](examples/syscalls/kernel_pid.c) | [kernel_pid.py](examples/syscalls/kernel_pid.py) | [KernelPid.java](examples/syscalls/KernelPid.java) |
| Syscalls | Copie os bytes de um arquivo para um destino novo. | [copy_file.c](examples/syscalls/copy_file.c) | [copy_file.py](examples/syscalls/copy_file.py) | [CopyFile.java](examples/syscalls/CopyFile.java) |
| Syscalls | Leia um byte a partir de um deslocamento. | [file_seek.c](examples/syscalls/file_seek.c) | [file_seek.py](examples/syscalls/file_seek.py) | [FileSeek.java](examples/syscalls/FileSeek.java) |
| Processos | Observe a memória separada do pai e do filho e o estado de saída. | [fork_wait.c](examples/processos/fork_wait.c) | [fork_wait.py](examples/processos/fork_wait.py) | [ProcessWait.java](examples/processos/ProcessWait.java) |
| Processos | Execute outro programa e confira o código de saída. | [fork_exec.c](examples/processos/fork_exec.c) | [fork_exec.py](examples/processos/fork_exec.py) | [ProcessExec.java](examples/processos/ProcessExec.java) |
| Processos | Envie uma mensagem entre processos por um pipe e leia até o fim. | [pipe_ipc.c](examples/processos/pipe_ipc.c) | [pipe_ipc.py](examples/processos/pipe_ipc.py) | [PipeIpc.java](examples/processos/PipeIpc.java) |
| Threads | Crie duas threads e leia seus resultados depois de esperar por elas. | [pthread_join.c](examples/threads/pthread_join.c) | [pthread_join.py](examples/threads/pthread_join.py) | [ThreadJoin.java](examples/threads/ThreadJoin.java) |
| Threads | Proteja um contador compartilhado com um mutex. | [mutex_counter.c](examples/threads/mutex_counter.c) | [mutex_counter.py](examples/threads/mutex_counter.py) | [MutexCounter.java](examples/threads/MutexCounter.java) |
| Threads | Coordene um produtor e um consumidor com variáveis de condição. | [condvar_queue.c](examples/threads/condvar_queue.c) | [condvar_queue.py](examples/threads/condvar_queue.py) | [CondvarQueue.java](examples/threads/CondvarQueue.java) |

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

Para executar os exemplos em Python, passe o caminho do arquivo a `python3`. Para Java, compile com `make java` e passe o nome da classe a `java`.

```sh
python3 examples/syscalls/kernel_pid.py
python3 examples/processos/fork_wait.py
python3 examples/threads/mutex_counter.py

make java
java -cp build/java KernelPid
java -cp build/java ProcessWait
java -cp build/java MutexCounter
```

## Conceitos para acompanhar o código

- **Syscalls.** Programas pedem serviços ao kernel. Aqui, `getpid()` usa o wrapper específico da libc e `syscall(SYS_getpid)` usa o wrapper genérico da mesma biblioteca. A função `open()` também é um wrapper, e em Linux pode aparecer como `openat` no rastreamento. [Manual de `syscall(2)`](https://man7.org/linux/man-pages/man2/syscall.2.html) · [Manual de `open(2)`](https://man7.org/linux/man-pages/man2/open.2.html)
- **Processos.** Depois de `fork()`, pai e filho têm espaços de memória separados. Descritores herdados podem referenciar a mesma descrição de arquivo do kernel, que contém o deslocamento. O pai usa `waitpid()` para obter o estado de saída do filho. [Manual de `fork(2)`](https://man7.org/linux/man-pages/man2/fork.2.html) · [Manual de `waitpid(2)`](https://man7.org/linux/man-pages/man2/waitpid.2.html)
- **Threads.** As threads compartilham a memória do processo. O mutex protege o acesso concorrente ao estado e a variável de condição permite esperar por uma mudança nesse estado. Funções `pthread_*` como `pthread_create` devolvem diretamente o código de erro. [Manual de `pthreads(7)`](https://man7.org/linux/man-pages/man7/pthreads.7.html) · [Manual de `pthread_cond_wait(3p)`](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html)

## Como as APIs se correspondem

- **Syscalls.** C demonstra a chamada explícita com `syscall(SYS_getpid)`. Python acessa descritores com funções de `os` e Java usa `FileChannel` para ler e posicionar arquivos. Os exemplos de PID em Python e Java conferem o valor com `/proc/self/stat`.
- **Processos.** Python usa `os.fork()`, `os.waitpid()` e `os.pipe()`. Em Java, `ProcessBuilder` inicia um subprocesso e `Process.waitFor()` espera seu término e retorna o código de saída. O exemplo de isolamento inicia uma segunda JVM, com sua própria memória. [Documentação de `os.fork()`](https://docs.python.org/3/library/os.html#os.fork) · [Documentação de `ProcessBuilder`](https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/lang/ProcessBuilder.html)
- **Threads.** Os exemplos em C chamam a API POSIX. Em Python, `threading` oferece threads, locks e variáveis de condição. Em Java, `Thread`, `ReentrantLock` e `Condition` oferecem as operações usadas no laboratório. [Documentação de `threading`](https://docs.python.org/3/library/threading.html) · [Documentação de `Condition`](https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/util/concurrent/locks/Condition.html)

Com o `strace` instalado, você pode acompanhar as syscalls dos exemplos. A opção `-f` segue filhos e threads.

```sh
strace -e trace=getpid ./build/syscalls/kernel_pid
strace -f -e trace=clone,clone3,fork,vfork,execve,wait4,waitid ./build/processos/fork_exec
strace -f -e trace=clone,clone3,futex ./build/threads/condvar_queue
```

Para praticar, siga o [roteiro de exercícios](docs/roteiro.md).
