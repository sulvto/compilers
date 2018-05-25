import base.Interpreter;
import org.junit.Test;
import stack.StackInterpreter;

import java.io.File;
import java.util.stream.Stream;

public class InterpreterTest {
    @Test
    public void main() throws Exception {

        Stream.of(new File("pie/pcode").listFiles()).forEach(file -> {
            try {
                System.out.println(file);
                String[] args = {file.getPath()};
                StackInterpreter.main(args);
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
    }
}