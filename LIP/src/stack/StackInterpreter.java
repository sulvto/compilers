package stack;

import assembler.BytecodeDefinition;
import assembler.FunctionSymbol;
import base.Interpreter;
import base.StructSpace;

/**
 * A simple stack-based interpreter
 * <p>
 * Created by sulvto on 17-10-30.
 */
public class StackInterpreter extends Interpreter {

    public static void main(String[] args) throws Exception {
        run(args, new StackInterpreter());
    }


    // Operand stack, grows,upwards
    Object[] operands = new Object[Interpreter.DEFAULT_OPERAND_STACK_SIZE];
    int sp = -1;            // stack pointer register

    // stack of stack frames, grows upwards
    protected StackFrame[] calls = new StackFrame[DEFAULT_CALL_STACK_SIZE];

    @Override
    protected void exec() {
        if (mainFunction == null) {
            mainFunction = new FunctionSymbol("main", 0, 0, 0);
        }

        StackFrame frame = new StackFrame(mainFunction, -1);
        calls[++fp] = frame;
        super.exec();
    }

    // Simulate the fetch-execute cycle
    public void cpu() {
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


    private Object pop() {
        return operands[sp--];
    }

    private void push(Object o) {
        operands[++sp] = o;
    }

}
