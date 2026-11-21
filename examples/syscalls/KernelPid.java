import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public final class KernelPid {
    public static void main(String[] args) {
        if (args.length != 0) {
            System.err.println("Uso: java KernelPid");
            System.exit(1);
        }

        // ProcessHandle é uma API da JVM. O procfs expõe o PID como texto.
        long javaPid = ProcessHandle.current().pid();
        long procPid;
        try {
            String procStat = Files.readString(Path.of("/proc/self/stat"));
            int separator = procStat.indexOf(' ');
            if (separator < 1) {
                throw new IOException("Campo de PID ausente.");
            }
            procPid = Long.parseLong(procStat.substring(0, separator));
        } catch (IOException | NumberFormatException error) {
            System.err.println("Erro ao consultar /proc/self/stat: " + error.getMessage());
            System.exit(1);
            return;
        }

        System.out.println("PID via ProcessHandle.current().pid(): " + javaPid);
        System.out.println("PID via /proc/self/stat: " + procPid);
        if (javaPid != procPid) {
            System.err.println("As duas consultas retornaram PIDs diferentes.");
            System.exit(1);
        }
    }
}
