#!/bin/sh
set -eu

temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
count=0
language=${1:-c}

case "$language" in
    c) language_label=C ;;
    python) language_label=Python ;;
    java) language_label=Java ;;
    *)
        printf 'Linguagem desconhecida: %s\n' "$language" >&2
        exit 1
        ;;
esac

pass() {
    count=$((count + 1))
    printf '✓ %s\n' "$1"
}

fail() {
    printf 'Falha: %s\n' "$1" >&2
    exit 1
}

case "$language" in
    python) python_executable=$(command -v "${PYTHON:-python3}") || fail 'Python 3 não encontrado' ;;
    java) java_executable=$(command -v "${JAVA:-java}") || fail 'Java não encontrado' ;;
esac

run_example() {
    example=$1
    class_name=$2
    shift 2
    case "$language" in
        c) set -- "./build/$example" "$@" ;;
        python) set -- "$python_executable" "examples/$example.py" "$@" ;;
        java) set -- "$java_executable" -cp build/java "$class_name" "$@" ;;
    esac
    timeout 5s env PATH="${example_path:-$PATH}" "$@"
}

expect_status() {
    expected=$1
    shift
    if run_example "$@" > "$temp_dir/stdout" 2> "$temp_dir/stderr"; then
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

expect_status 0 syscalls/kernel_pid KernelPid
awk '
    /^PID via .+: [1-9][0-9]*$/ { pid[NR] = $NF }
    END { exit !(NR == 2 && pid[1] > 0 && pid[1] == pid[2]) }
' "$temp_dir/stdout" || fail 'as duas APIs precisam retornar o mesmo PID positivo'
pass 'comparação de PIDs'

printf '\000\377C' > "$temp_dir/origem binaria"
dd if=/dev/zero bs=4096 count=5 >> "$temp_dir/origem binaria" 2> /dev/null
printf '\001\376' >> "$temp_dir/origem binaria"
expect_status 0 syscalls/copy_file CopyFile "$temp_dir/origem binaria" "$temp_dir/copia"
cmp "$temp_dir/origem binaria" "$temp_dir/copia" || fail 'a cópia precisa preservar todos os bytes'
pass 'cópia binária em mais de um bloco'

: > "$temp_dir/vazio"
expect_status 0 syscalls/copy_file CopyFile "$temp_dir/vazio" "$temp_dir/copia vazia"
cmp "$temp_dir/vazio" "$temp_dir/copia vazia" || fail 'a cópia do arquivo vazio precisa ficar vazia'
pass 'cópia de arquivo vazio'

printf 'preservar\n' > "$temp_dir/destino existente"
expect_status 1 syscalls/copy_file CopyFile "$temp_dir/vazio" "$temp_dir/destino existente"
[ "$(cat "$temp_dir/destino existente")" = preservar ] || fail 'o destino existente precisa ser preservado'
pass 'recusa de sobrescrita'

expect_status 1 syscalls/copy_file CopyFile "$temp_dir/ausente" "$temp_dir/nao criado"
[ ! -e "$temp_dir/nao criado" ] || fail 'origem ausente não deve criar destino'
pass 'cópia com origem ausente'

mkfifo "$temp_dir/fifo"
expect_status 1 syscalls/copy_file CopyFile "$temp_dir/fifo" "$temp_dir/nao criado"
[ ! -e "$temp_dir/nao criado" ] || fail 'um FIFO não deve criar destino'
pass 'recusa de origem não regular'

printf 'ABC\n' > "$temp_dir/bytes"
expect_output 'Byte na posição 1: 0x42' syscalls/file_seek FileSeek "$temp_dir/bytes" 1
expect_output 'Byte na posição 0: 0x00' syscalls/file_seek FileSeek "$temp_dir/origem binaria" 0
pass 'leitura de um byte a partir do deslocamento'

expect_status 1 syscalls/file_seek FileSeek "$temp_dir/bytes" 4
expect_status 1 syscalls/file_seek FileSeek "$temp_dir/bytes" -1
expect_status 1 syscalls/file_seek FileSeek "$temp_dir/fifo" 0
pass 'fim de arquivo, deslocamento negativo e descritor sem suporte a lseek'

expect_output 'O filho alterou sua cópia do valor para 20.
O pai ainda tem o valor 10.
O filho terminou com código 7.' processos/fork_wait ProcessWait
pass 'memória isolada e estado de saída do filho'

expect_output 'O filho agora executa o comando echo.
O pai recebeu o código de saída 0.' processos/fork_exec ProcessExec
mkdir "$temp_dir/sem comandos"
if [ "$language" = java ]; then
    example_path="$temp_dir/sem comandos" expect_status 1 processos/fork_exec ProcessExec
    pass 'execução de outro programa e falha de inicialização'
else
    example_path="$temp_dir/sem comandos" expect_status 127 processos/fork_exec ProcessExec
    pass 'substituição do processo e falha de exec'
fi

expect_output 'O pai recebeu "Olá pelo pipe!" antes do fim do pipe.' processos/pipe_ipc PipeIpc
pass 'mensagem completa e fechamento do pipe'

expect_output 'Thread 1: 6 x 6 = 36
Thread 2: 7 x 7 = 49' threads/pthread_join ThreadJoin
pass 'criação e conclusão das threads'

expect_output 'Contador final: 400000 (esperado: 400000)' threads/mutex_counter MutexCounter
pass 'contador protegido pelo mutex'

expect_output 'Itens consumidos: 5, soma: 15 (esperada: 15)' threads/condvar_queue CondvarQueue
pass 'fila com produção, consumo e término'

printf '\n%s verificações passaram em %s.\n' "$count" "$language_label"
