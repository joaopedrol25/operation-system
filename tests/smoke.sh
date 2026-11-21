#!/bin/sh
set -eu

temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
count=0

pass() {
    count=$((count + 1))
    printf '✓ %s\n' "$1"
}

fail() {
    printf 'Falha: %s\n' "$1" >&2
    exit 1
}

expect_status() {
    expected=$1
    shift
    if timeout 5s "$@" > "$temp_dir/stdout" 2> "$temp_dir/stderr"; then
        status=0
    else
        status=$?
    fi
    if [ "$status" -ne "$expected" ]; then
        printf 'Comando: %s\nStatus esperado: %s. Recebido: %s.\n' "$*" "$expected" "$status" >&2
        cat "$temp_dir/stdout" "$temp_dir/stderr" >&2
        exit 1
    fi
}

expect_output() {
    expected_output=$1
    shift
    expect_status 0 "$@"
    if [ "$(cat "$temp_dir/stdout")" != "$expected_output" ]; then
        printf 'Comando: %s\nSaída esperada:\n%s\nSaída recebida:\n' "$*" "$expected_output" >&2
        cat "$temp_dir/stdout" >&2
        exit 1
    fi
}

expect_status 0 ./build/syscalls/kernel_pid
awk '
    /^PID via libc getpid\(\): [1-9][0-9]*$/ { libc = $NF }
    /^PID via syscall\(SYS_getpid\): [1-9][0-9]*$/ { kernel = $NF }
    END { exit !(NR == 2 && libc > 0 && libc == kernel) }
' "$temp_dir/stdout" || fail 'getpid e syscall precisam retornar o mesmo PID positivo'
pass 'PID da libc e da syscall'

printf '\000\377C' > "$temp_dir/origem binaria"
dd if=/dev/zero bs=4096 count=5 >> "$temp_dir/origem binaria" 2> /dev/null
printf '\001\376' >> "$temp_dir/origem binaria"
expect_status 0 ./build/syscalls/copy_file "$temp_dir/origem binaria" "$temp_dir/copia"
cmp "$temp_dir/origem binaria" "$temp_dir/copia" || fail 'a cópia precisa preservar todos os bytes'
pass 'cópia binária em mais de um bloco'

: > "$temp_dir/vazio"
expect_status 0 ./build/syscalls/copy_file "$temp_dir/vazio" "$temp_dir/copia vazia"
cmp "$temp_dir/vazio" "$temp_dir/copia vazia" || fail 'a cópia do arquivo vazio precisa ficar vazia'
pass 'cópia de arquivo vazio'

printf 'preservar\n' > "$temp_dir/destino existente"
expect_status 1 ./build/syscalls/copy_file "$temp_dir/vazio" "$temp_dir/destino existente"
[ "$(cat "$temp_dir/destino existente")" = preservar ] || fail 'o destino existente precisa ser preservado'
pass 'recusa de sobrescrita'

expect_status 1 ./build/syscalls/copy_file "$temp_dir/ausente" "$temp_dir/nao criado"
[ ! -e "$temp_dir/nao criado" ] || fail 'origem ausente não deve criar destino'
pass 'cópia com origem ausente'

mkfifo "$temp_dir/fifo"
expect_status 1 ./build/syscalls/copy_file "$temp_dir/fifo" "$temp_dir/nao criado"
[ ! -e "$temp_dir/nao criado" ] || fail 'um FIFO não deve criar destino'
pass 'recusa de origem não regular'

printf 'ABC\n' > "$temp_dir/bytes"
expect_output 'Byte na posição 1: 0x42' ./build/syscalls/file_seek "$temp_dir/bytes" 1
expect_output 'Byte na posição 0: 0x00' ./build/syscalls/file_seek "$temp_dir/origem binaria" 0
pass 'leitura de um byte a partir do deslocamento'

expect_status 1 ./build/syscalls/file_seek "$temp_dir/bytes" 4
expect_status 1 ./build/syscalls/file_seek "$temp_dir/bytes" -1
expect_status 1 ./build/syscalls/file_seek "$temp_dir/fifo" 0
pass 'fim de arquivo, deslocamento negativo e descritor sem suporte a lseek'

expect_output 'O filho alterou sua cópia do valor para 20.
O pai ainda tem o valor 10.
O filho terminou com código 7.' ./build/processos/fork_wait
pass 'memória isolada e estado de saída do filho'

expect_output 'O filho agora executa o comando echo.
O pai recebeu o código de saída 0.' ./build/processos/fork_exec
mkdir "$temp_dir/sem comandos"
expect_status 127 env PATH="$temp_dir/sem comandos" ./build/processos/fork_exec
pass 'substituição do processo e falha de exec'

expect_output 'O pai recebeu "Olá pelo pipe!" antes do fim do pipe.' ./build/processos/pipe_ipc
pass 'mensagem completa e fechamento do pipe'

expect_output 'Thread 1: 6 x 6 = 36
Thread 2: 7 x 7 = 49' ./build/threads/pthread_join
pass 'criação e conclusão das threads'

expect_output 'Contador final: 400000 (esperado: 400000)' ./build/threads/mutex_counter
pass 'contador protegido pelo mutex'

expect_output 'Itens consumidos: 5, soma: 15 (esperada: 15)' ./build/threads/condvar_queue
pass 'fila com produção, consumo e término'

printf '\n%s verificações passaram.\n' "$count"
