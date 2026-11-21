import java.io.IOException;

public class ProcessExec {
    public static void main(String[] args) {
        try {
            System.exit(executar());
        } catch (IOException erro) {
            System.err.println("Falha ao iniciar echo. " + erro.getMessage());
            System.exit(1);
        } catch (InterruptedException erro) {
            Thread.currentThread().interrupt();
            System.err.println("A espera pelo comando foi interrompida.");
            System.exit(1);
        }
    }

    private static int executar() throws IOException, InterruptedException {
        // ProcessBuilder solicita o subprocesso com o comando e os argumentos.
        Process comando = new ProcessBuilder("echo", "O filho agora executa o comando echo.")
                .inheritIO()
                .start();

        int codigo;
        try {
            codigo = comando.waitFor();
        } catch (InterruptedException erro) {
            comando.destroyForcibly();
            throw erro;
        }

        System.out.printf("O pai recebeu o código de saída %d.%n", codigo);
        return codigo;
    }
}
