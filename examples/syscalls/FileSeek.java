import java.io.EOFException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Locale;

public final class FileSeek {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Uso: java FileSeek ARQUIVO DESLOCAMENTO");
            System.exit(1);
        }

        long offset;
        try {
            offset = parseOffset(args[1]);
        } catch (NumberFormatException error) {
            System.err.println("DESLOCAMENTO precisa ser um inteiro decimal não negativo dentro do limite do sistema.");
            System.exit(1);
            return;
        }

        try {
            int result = byteAt(Path.of(args[0]), offset);
            System.out.printf(Locale.ROOT, "Byte na posição %d: 0x%02X%n", offset, result);
        } catch (IOException | InvalidPathException error) {
            System.err.println(error.getMessage());
            System.exit(1);
        }
    }

    private static long parseOffset(String text) {
        if (text.isEmpty() || !text.chars().allMatch(character -> character >= '0' && character <= '9')) {
            throw new NumberFormatException();
        }
        return Long.parseLong(text);
    }

    private static int byteAt(Path path, long offset) throws IOException {
        // NIO trabalha com canais. Verificamos os atributos antes de abrir um possível FIFO.
        if (!Files.readAttributes(path, BasicFileAttributes.class).isRegularFile()) {
            throw new IOException("A entrada precisa ser um arquivo regular com suporte a posicionamento.");
        }

        ByteBuffer buffer = ByteBuffer.allocate(1);
        try (FileChannel file = FileChannel.open(path, StandardOpenOption.READ)) {
            file.position(offset);
            int count = file.read(buffer);
            if (count == -1) {
                throw new EOFException("Fim do arquivo na posição " + offset + ".");
            }
            if (count == 0) {
                throw new IOException("A leitura do arquivo não avançou.");
            }
            return Byte.toUnsignedInt(buffer.get(0));
        }
    }
}
