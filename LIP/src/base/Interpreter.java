package base;

import assembler.BytecodeAssembler;
import assembler.BytecodeDefinition;
import assembler.FunctionSymbol;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * A simple stack-based interpreter
 * <p>
 * Created by sulvto on 17-10-29.
 */
public abstract class Interpreter {
    public static final int DEFAULT_OPERAND_STACK_SIZE = 100;
    public static final int DEFAULT_CALL_STACK_SIZE = 1000;

    protected int ip;
    protected byte[] code;
    protected int codeSize;
    protected Object[] globals;       // global variable space
    protected Object[] constPool;

    protected int fp = -1;            // frame pointer register

    protected FunctionSymbol mainFunction;

    protected boolean trace = false;

    public static void run(String[] args, Interpreter interpreter) throws IOException {

        boolean trace = false;
        boolean disassemble = false;
        boolean dump = false;
        String filename = null;
        int i = 0;
        while (i < args.length) {
            if (args[i].equals("-trace")) trace = true;
            else if (args[i].equals("-dis")) disassemble = true;
            else if (args[i].equals("-dump")) dump = true;
            else filename = args[i];
            i++;
        }
        InputStream input = filename == null ? System.in : new FileInputStream(filename);

        load(interpreter, input);

        interpreter.trace = trace;
        interpreter.exec();
        if (disassemble) interpreter.disassemble();
        if (dump) interpreter.coredump();

    }

    protected static void load(Interpreter interpreter, InputStream input) throws IOException {
        try {
            BytecodeAssembler assembler = new BytecodeAssembler(input, BytecodeDefinition.instructions);
            assembler.program();
            interpreter.code = assembler.getMachineCode();
            interpreter.codeSize = assembler.getCodeMemorySize();
            interpreter.constPool = assembler.getConstantPool();
            interpreter.mainFunction = assembler.getMainFunction();
            interpreter.globals = new Object[assembler.getDataSize()];
//            interpreter.disasm = new DisAssembler(interpreter.code, interpreter.codeSize, interpreter.constPool);
        } finally {
            input.close();
        }
    }

    protected void exec() {

        ip = mainFunction.getAddress();
        cpu();
    }

    // Simulate the fetch-execute cycle
    protected abstract void cpu();

    protected int getIntOperand() {
        int word = BytecodeAssembler.getInt(code, ip);
        ip += 4;
        return word;
    }

    protected void trace() {
    }

    protected void disassemble() {
    }

    protected void coredump() {
    }


}
