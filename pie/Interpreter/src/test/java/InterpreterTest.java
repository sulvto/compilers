package test.java;

import jdk.internal.org.objectweb.asm.tree.analysis.Interpreter;
import org.junit.Test;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.stream.Stream;

/**
 * Created by sulvto on 17-10-28.
 */
public class InterpreterTest {
    @Test
    public void run() throws Exception {
        Stream.of(new File("src/main/pie").listFiles()).forEach(file -> {
            try {
                System.out.println(file);
                new Interpreter().interp(new FileInputStream(file));
            } catch (IOException e) {
                e.printStackTrace();
            }
        });

    }
}