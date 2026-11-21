import java.io.IOException;
import java.nio.file.Path;

public class ProcessWait {
    private static final int CODIGO_DO_FILHO = 7;

    public static void main(String[] args) {
        int valor = 10;

        if (args.length == 1 && "--filho".equals(args[0])) {
            valor = 20;
            System.out.printf("O filho alterou sua cópia do valor para %d.%n", valor);
            System.exit(System.out.checkError() ? 1 : CODIGO_DO_FILHO);
        }

        try {
            System.exit(executarPai(valor));
        } catch (IOException erro) {
            System.err.println("Falha ao iniciar o filho. " + erro.getMessage());
            System.exit(1);
        } catch (InterruptedException erro) {
            Thread.currentThread().interrupt();
            System.err.println("A espera pelo filho foi interrompida.");
            System.exit(1);
        }
    }

    private static int executarPai(int valor) throws IOException, InterruptedException {
        String java = Path.of(System.getProperty("java.home"), "bin", "java").toString();

        // A JVM do filho tem sua própria memória e executa a mesma classe.
        Process filho = new ProcessBuilder(
                java, "-cp", System.getProperty("java.class.path"),
                ProcessWait.class.getName(), "--filho")
                .inheritIO()
                .start();

        int codigo;
        try {
            codigo = filho.waitFor();
        } catch (InterruptedException erro) {
            filho.destroyForcibly();
            throw erro;
        }

        System.out.printf("O pai ainda tem o valor %d.%n", valor);
        System.out.printf("O filho terminou com código %d.%n", codigo);
        return codigo == CODIGO_DO_FILHO ? 0 : 1;
    }
}
