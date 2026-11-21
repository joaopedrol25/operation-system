import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;

public class PipeIpc {
    public static void main(String[] args) {
        if (args.length == 1 && "--filho".equals(args[0])) {
            System.out.writeBytes("Olá pelo pipe!".getBytes(StandardCharsets.UTF_8));
            System.exit(System.out.checkError() ? 1 : 0);
        }

        try {
            System.exit(executarPai());
        } catch (IOException erro) {
            System.err.println("Falha no subprocesso ou no pipe. " + erro.getMessage());
            System.exit(1);
        } catch (InterruptedException erro) {
            Thread.currentThread().interrupt();
            System.err.println("A espera pelo filho foi interrompida.");
            System.exit(1);
        }
    }

    private static int executarPai() throws IOException, InterruptedException {
        String java = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        Process filho = new ProcessBuilder(
                java, "-cp", System.getProperty("java.class.path"),
                PipeIpc.class.getName(), "--filho")
                .redirectOutput(ProcessBuilder.Redirect.PIPE)
                .redirectError(ProcessBuilder.Redirect.INHERIT)
                .start();

        try (InputStream entrada = filho.getInputStream()) {
            filho.getOutputStream().close();
            ByteArrayOutputStream mensagem = new ByteArrayOutputStream();
            byte[] bloco = new byte[8];
            int lidos;

            // O fechamento da saída pelo filho produz EOF no lado do pai.
            while ((lidos = entrada.read(bloco)) != -1) {
                mensagem.write(bloco, 0, lidos);
            }

            if (filho.waitFor() != 0) {
                System.err.println("O filho falhou ao enviar a mensagem.");
                return 1;
            }

            String texto = mensagem.toString(StandardCharsets.UTF_8);
            System.out.printf("O pai recebeu \"%s\" antes do fim do pipe.%n", texto);
            return 0;
        } finally {
            if (filho.isAlive()) {
                filho.destroyForcibly();
            }
        }
    }
}
