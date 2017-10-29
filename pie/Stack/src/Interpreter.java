
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
public class Interpreter {
    public static final int DEFAULT_OPERAND_STACK_SIZE = 100;
    public static final int DEFAULT_CALL_STACK_SIZE = 1000;

    int ip;
    byte[] code;
    int codeSize;
    Object[] globals;       // global variable space
    protected Object[] constPool;

    // Operand stack, grows,upwards
    Object[] operands = new Object[DEFAULT_OPERAND_STACK_SIZE];
    int sp = -1;            // stack pointer register

    // Stack of stack frames, grows upwards
    StackFrame[] calls = new StackFrame[DEFAULT_CALL_STACK_SIZE];
    int fp = -1;            // frame pointer register

    FunctionSymbol mainFunction;

    boolean trace = false;

    public static void main(String[] args) throws Exception {
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

        Interpreter interpreter = new Interpreter();
        init(interpreter, input);
        interpreter.trace = trace;
        interpreter.exec();
        if (disassemble) interpreter.disassemble();
        if (dump) interpreter.coredump();
    }

    private static void init(Interpreter interpreter, InputStream input) throws IOException {
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

    private void exec() {
        if (mainFunction == null) {
            mainFunction = new FunctionSymbol("main", 0, 0, 0);
        }

        StackFrame frame = new StackFrame(mainFunction, -1);
        calls[++fp] = frame;
        ip = mainFunction.getAddress();
        cpu();
    }

    // Simulate the fetch-execute cycle
    private void cpu() {
        Object v = null; // some locals to reuse
        int a, b;
        float e, f;
        int addr = 0;
        short opcode = code[ip];
        while (opcode != BytecodeDefinition.INSTR_HALT && ip < codeSize) {
            if (trace) trace();
            ip++;
            switch (opcode) {
                case BytecodeDefinition.INSTR_IADD:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a + b);
                    break;
                case BytecodeDefinition.INSTR_ISUB:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a - b);
                    break;
                case BytecodeDefinition.INSTR_IMUL:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a * b);
                    break;
                case BytecodeDefinition.INSTR_IDIV:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a / b);
                    break;
                case BytecodeDefinition.INSTR_ILT:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a < b);
                    break;
                case BytecodeDefinition.INSTR_IGT:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a > b);
                    break;
                case BytecodeDefinition.INSTR_IEQ:
                    a = (Integer) operands[sp - 1];
                    b = (Integer) operands[sp];
                    sp -= 2;
                    push(a == b);
                    break;
                case BytecodeDefinition.INSTR_FADD:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e + f);
                    break;
                case BytecodeDefinition.INSTR_FSUB:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e - f);
                    break;
                case BytecodeDefinition.INSTR_FMUL:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e * f);
                    break;
                case BytecodeDefinition.INSTR_FDIV:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e / f);
                    break;
                case BytecodeDefinition.INSTR_FLT:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e < f);
                    break;
                case BytecodeDefinition.INSTR_FGT:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e > f);
                    break;
                case BytecodeDefinition.INSTR_FEQ:
                    e = (Float) operands[sp - 1];
                    f = (Float) operands[sp];
                    sp -= 2;
                    push(e == f);
                    break;
                case BytecodeDefinition.INSTR_ITOF:
                    a = (Integer) pop();
                    push((float) a);
                    break;
                case BytecodeDefinition.INSTR_CALL:
                    int funcIndexInConstPoolIndex = getIntOperand();
                    call(funcIndexInConstPoolIndex);
                    break;
                case BytecodeDefinition.INSTR_RET:
                    StackFrame frame = calls[fp--];
                    ip = frame.returnAddress;
                    break;
                case BytecodeDefinition.INSTR_BR:
                    ip = getIntOperand();
                    break;
                case BytecodeDefinition.INSTR_BRT:
                    addr = getIntOperand();
                    if (pop().equals(true)) ip = addr;
                    break;
                case BytecodeDefinition.INSTR_BRF:
                    addr = getIntOperand();
                    if (pop().equals(false)) ip = addr;
                    break;
                case BytecodeDefinition.INSTR_CCONST:
                    push((char) getIntOperand());
                    break;
                case BytecodeDefinition.INSTR_ICONST:
                    push(getIntOperand());
                    break;
                case BytecodeDefinition.INSTR_FCONST:
                case BytecodeDefinition.INSTR_SCONST:
                    int constPoolIndex = getIntOperand();
                    push(constPool[constPoolIndex]);
                    break;
                case BytecodeDefinition.INSTR_LOAD:
                    addr = getIntOperand();
                    push(calls[fp].locals[addr]);
                    break;
                case BytecodeDefinition.INSTR_GLOAD:
                    addr = getIntOperand();
                    push(globals[addr]);
                    break;
                case BytecodeDefinition.INSTR_FLOAD:
                    StructSpace struct = (StructSpace) pop();
                    int fieldOffset = getIntOperand();
                    push(struct.fields[fieldOffset]);
                    break;
                case BytecodeDefinition.INSTR_STORE:
                    addr = getIntOperand();
                    calls[fp].locals[addr] = pop();
                    break;
                case BytecodeDefinition.INSTR_GSTORE:
                    addr = getIntOperand();
                    globals[addr] = pop();
                    break;
                case BytecodeDefinition.INSTR_FSTORE:
                    struct = (StructSpace) pop();
                    v = pop();
                    fieldOffset = getIntOperand();
                    struct.fields[fieldOffset] = v;
                    break;
                case BytecodeDefinition.INSTR_PRINT:
                    System.out.println(pop());
                    break;
                case BytecodeDefinition.INSTR_STRUCT:
                    int nfields = getIntOperand();
                    push(new StructSpace(nfields));
                    break;
                case BytecodeDefinition.INSTR_NULL:
                    push(null);
                    break;
                case BytecodeDefinition.INSTR_POP:
                    pop();
                    break;
                default:
                    throw new Error("invalid opcode: " + opcode + " at ip=" + (ip - 1));
            }
            opcode = code[ip];
        }
    }

    private void call(int funcIndexInConstPoolIndex) {
        FunctionSymbol fs = (FunctionSymbol) constPool[funcIndexInConstPoolIndex];
        StackFrame frame = new StackFrame(fs, ip);
        calls[++fp] = frame;
        for (int a = fs.getNargs() - 1; a >= 0; a--) {
            frame.locals[a] = pop();
        }
        ip = fs.getAddress(); // branch to function
    }

    public int getIntOperand() {
        int word = BytecodeAssembler.getInt(code, ip);
        ip += 4;
        return word;
    }

    private Object pop() {
        return operands[sp--];
    }

    private void push(Object o) {
        operands[++sp] = o;
    }

    private void trace() {
    }

    private void disassemble() {
    }

    private void coredump() {
    }


}
