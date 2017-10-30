package reg;

import assembler.BytecodeDefinition;
import assembler.FunctionSymbol;
import base.Interpreter;
import base.StructSpace;

/**
 * A simple register-based interpreter.
 * Created by sulvto on 17-10-30.
 */
public class RegInterpreter extends Interpreter {
    /**
     * Stack of stack frames, grows upwards
     */
    StackFrame[] calls = new StackFrame[DEFAULT_CALL_STACK_SIZE];
    int fp = -1;        // frame pointer register

    public static void main(String[] args) throws Exception {
        run(args, new RegInterpreter());
    }

    @Override

    protected void exec() {
        if (mainFunction == null) {
            mainFunction = new FunctionSymbol("main", 0, 0, 0);
        }

        StackFrame frame = new StackFrame(mainFunction, ip);
        calls[++fp] = frame;
        super.exec();
    }

    @Override
    protected void cpu() {
        Object v = null; // some locals to reuse
        int i = 0, j = 0, k = 0, addr = 0, fieldIndex = 0;

        short opcode = code[ip];

        while (opcode != BytecodeDefinition.INSTR_HALT && ip < codeSize) {
            if (trace) trace();
            ip++;  // jump next instruction
            Object reg[] = calls[fp].register;
            switch (opcode) {
                case BytecodeDefinition.INSTR_IADD:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Integer) reg[i] + (Integer) reg[j];
                    break;
                case BytecodeDefinition.INSTR_ISUB:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Integer) reg[i] - (Integer) reg[j];
                    break;
                case BytecodeDefinition.INSTR_IMUL:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Integer) reg[i] * (Integer) reg[j];
                    break;
                case BytecodeDefinition.INSTR_IDIV:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Integer) reg[i] / (Integer) reg[j];
                    break;
                case BytecodeDefinition.INSTR_ILT:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Integer) reg[i]).intValue() < ((Integer) reg[j]).intValue();
                    break;
                case BytecodeDefinition.INSTR_IGT:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Integer) reg[i]).intValue() > ((Integer) reg[j]).intValue();
                    break;
                case BytecodeDefinition.INSTR_IEQ:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Integer) reg[i]).intValue() == ((Integer) reg[j]).intValue();
                    break;
                case BytecodeDefinition.INSTR_FADD:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Float) reg[i] + (Float) reg[j];
                    break;
                case BytecodeDefinition.INSTR_FSUB:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Float) reg[i] - (Float) reg[j];
                    break;
                case BytecodeDefinition.INSTR_FMUL:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Float) reg[i] * (Float) reg[j];
                    break;
                case BytecodeDefinition.INSTR_FDIV:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = (Float) reg[i] / (Float) reg[j];
                    break;
                case BytecodeDefinition.INSTR_FLT:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Float) reg[i]).floatValue() < ((Float) reg[j]).floatValue();
                    break;
                case BytecodeDefinition.INSTR_FGT:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Float) reg[i]).floatValue() > ((Float) reg[j]).floatValue();
                    break;
                case BytecodeDefinition.INSTR_FEQ:
                    i = getRegOperand();
                    j = getRegOperand();
                    k = getRegOperand();
                    reg[k] = ((Float) reg[i]).floatValue() == ((Float) reg[j]).floatValue();
                    break;
                case BytecodeDefinition.INSTR_ITOF:
                    i = getRegOperand();
                    j = getRegOperand();
                    reg[j] = (float) ((Integer) reg[i]).intValue();
                    break;
                case BytecodeDefinition.INSTR_CALL:
                    int funcStringIndex = getIntOperand();
                    int baseRegisterIndex = getRegOperand();
                    call(funcStringIndex, baseRegisterIndex);
                    break;
                case BytecodeDefinition.INSTR_RET:
                    StackFrame frame = calls[fp--];
                    calls[fp].register[0] = frame.register[0];
                    ip = frame.returnAddress;
                    break;
                case BytecodeDefinition.INSTR_BR:
                    ip = getIntOperand();
                    break;
                case BytecodeDefinition.INSTR_BRT:
                    i = getRegOperand();
                    addr = getIntOperand();
                    Boolean bool = (Boolean) reg[i];
                    if (bool) ip = addr;
                    break;
                case BytecodeDefinition.INSTR_BRF:
                    i = getRegOperand();
                    addr = getIntOperand();
                    Boolean bool2 = (Boolean) reg[i];
                    if (!bool2) ip = addr;
                    break;
                case BytecodeDefinition.INSTR_CCONST:
                    i = getRegOperand();
                    reg[i] = (char) getIntOperand();
                    break;
                case BytecodeDefinition.INSTR_ICONST:
                    i = getRegOperand();
                    reg[i] = getIntOperand();
                    break;
                case BytecodeDefinition.INSTR_FCONST:
                case BytecodeDefinition.INSTR_SCONST:
                    i = getRegOperand();
                    reg[i] = constPool[getIntOperand()];
                    break;

                case BytecodeDefinition.INSTR_GLOAD:
                    i = getRegOperand();
                    addr = getIntOperand();
                    reg[i] = globals[addr];
                    break;
                case BytecodeDefinition.INSTR_FLOAD:
                    i = getRegOperand();
                    j = getRegOperand();
                    int fieldOffset = getIntOperand();
                    reg[i] = ((StructSpace) reg[j]).fields[fieldOffset];
                    break;
                case BytecodeDefinition.INSTR_GSTORE:
                    i = getRegOperand();
                    addr = getIntOperand();
                    globals[addr] = reg[i];
                    break;
                case BytecodeDefinition.INSTR_FSTORE:
                    i = getRegOperand();
                    j = getRegOperand();
                    fieldOffset = getIntOperand();
                    ((StructSpace) reg[j]).fields[fieldOffset] = reg[i];

                    break;
                case BytecodeDefinition.INSTR_PRINT:
                    System.out.println(reg[getRegOperand()]);
                    break;
                case BytecodeDefinition.INSTR_STRUCT:
                    i = getRegOperand();
                    int nfields = getIntOperand();
                    reg[i] = new StructSpace(nfields);
                    break;
                case BytecodeDefinition.INSTR_NULL:
                    i = getRegOperand();
                    reg[i] = null;
                    break;
                case BytecodeDefinition.INSTR_MOV:
                    i = getRegOperand();
                    j = getRegOperand();
                    reg[j] = reg[i];
                    break;
                default:
                    throw new Error("invalid opcode: " + opcode + " at ip=" + (ip - 1));
            }
            opcode = code[ip];
        }
    }

    private void call(int funcStringIndex, int baseRegisterIndex) {
        FunctionSymbol function = (FunctionSymbol) constPool[funcStringIndex];
        StackFrame frame = new StackFrame(function, ip);
        StackFrame callingFrame = calls[fp];
        calls[++fp] = frame;
        // move arguments from calling stack frame to new stack frame
        for (int a = 0; a < function.getNargs(); a++) {
            frame.register[a] = callingFrame.register[baseRegisterIndex + a];
        }
        ip = function.getAddress(); // branch to function
    }

    public int getRegOperand() {
        return getIntOperand();
    }
}
