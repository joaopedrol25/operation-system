import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;

public final class CopyFile {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Uso: java CopyFile ORIGEM DESTINO");
            System.exit(1);
        }

        try {
            copyBytes(Path.of(args[0]), Path.of(args[1]));
        } catch (IOException | InvalidPathException error) {
            System.err.println("Erro ao copiar: " + error.getMessage());
            for (Throwable cleanupError : error.getSuppressed()) {
                System.err.println("Erro ao encerrar a cópia: " + cleanupError.getMessage());
            }
            System.exit(1);
        }
    }

    private static void copyBytes(Path source, Path destination) throws IOException {
        // A API NIO não expõe O_NONBLOCK. A origem precisa permanecer estável até a abertura.
        if (!Files.readAttributes(source, BasicFileAttributes.class).isRegularFile()) {
            throw new IOException("A origem '" + source + "' precisa ser um arquivo regular.");
        }

        boolean created = false;
        try (FileChannel input = FileChannel.open(source, StandardOpenOption.READ)) {
            // CREATE_NEW verifica e cria o caminho de destino atomicamente.
            FileChannel output = FileChannel.open(destination,
                    StandardOpenOption.WRITE, StandardOpenOption.CREATE_NEW);
            created = true;
            try (output) {
                ByteBuffer buffer = ByteBuffer.allocate(16 * 1024);
                while (input.read(buffer) != -1) {
                    buffer.flip();
                    while (buffer.hasRemaining()) {
                        if (output.write(buffer) == 0) {
                            throw new IOException("A escrita no destino não avançou.");
                        }
                    }
                    buffer.clear();
                }
            }
        } catch (IOException error) {
            // O chamador controla o diretório de destino durante a cópia.
            if (created) {
                try {
                    Files.deleteIfExists(destination);
                } catch (IOException cleanupError) {
                    error.addSuppressed(cleanupError);
                }
            }
            throw error;
        }
    }
}
