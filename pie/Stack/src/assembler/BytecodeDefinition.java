package assembler;

/**
 * Created by sulvto on 17-10-29.
 */
public class BytecodeDefinition {
    public static final int REG = Type.REG;
    public static final int FUNC = Type.FUNC;
    public static final int INT = Type.INT;
    public static final int POOL = 1000;

    public static class Instruction {
        String name;
        int[] type = new int[3];
        int n = 0;

        public Instruction(String name) {
            this(name, 0, 0, 0);
            n = 0;
        }

        public Instruction(String name, int a) {
            this(name, a, 0, 0);
            n = 1;
        }

        public Instruction(String name, int a, int b) {
            this(name, a, b, 0);
            n = 2;
        }

        public Instruction(String name, int a, int b, int c) {
            this.name = name;
            type[0] = a;
            type[1] = b;
            type[2] = c;
            n = 3;
        }
    }

    // INSTRUCTION BYTECODES
    public static final short INSTR_IADD = 1;    // int add
    public static final short INSTR_ISUB = 2;
    public static final short INSTR_IMUL = 3;
    public static final short INSTR_IDIV = 4;
    public static final short INSTR_ILT = 5;     // int less then
    public static final short INSTR_IGT = 6;
    public static final short INSTR_IEQ = 7;
    public static final short INSTR_FADD = 8;    // float add
    public static final short INSTR_FSUB = 9;
    public static final short INSTR_FMUL = 10;
    public static final short INSTR_FDIV = 11;
    public static final short INSTR_FLT = 12;    // float less then
    public static final short INSTR_FGT = 13;
    public static final short INSTR_FEQ = 14;
    public static final short INSTR_ITOF = 15;   // int to float
    public static final short INSTR_CALL = 16;
    public static final short INSTR_RET = 17;
    public static final short INSTR_BR = 18;     // branch
    public static final short INSTR_BRT = 19;    // branch if true
    public static final short INSTR_BRF = 20;    // branch if false
    public static final short INSTR_CCONST = 21; // push constant char
    public static final short INSTR_ICONST = 22; // push constant integer
    public static final short INSTR_FCONST = 23; // push constant float
    public static final short INSTR_SCONST = 24; // push constant string
    public static final short INSTR_LOAD = 25;   // load from local context
    public static final short INSTR_GLOAD = 26;  // load from global memory
    public static final short INSTR_FLOAD = 27;  // field load
    public static final short INSTR_STORE = 28;  // store local context
    public static final short INSTR_GSTORE = 29; //
    public static final short INSTR_FSTORE = 30; //
    public static final short INSTR_PRINT = 31;  // print stack top
    public static final short INSTR_STRUCT = 32; // push new struct on stack
    public static final short INSTR_NULL = 33;   // push null onto stack
    public static final short INSTR_POP = 34;    // throw away top of stack
    public static final short INSTR_HALT = 35;


    public static Instruction[] instructions = new Instruction[]{
            null, // <INVALID>
            new Instruction("iadd"),  // index is the opcode
            new Instruction("isub"),
            new Instruction("imul"),
            new Instruction("idiv"),
            new Instruction("ilt"),
            new Instruction("igt"),
            new Instruction("ieq"),
            new Instruction("fadd"),
            new Instruction("fsub"),
            new Instruction("fmul"),
            new Instruction("fdiv"),
            new Instruction("flt"),
            new Instruction("fgt"),
            new Instruction("feq"),
            new Instruction("itof"),
            new Instruction("call", FUNC),
            new Instruction("ret"),
            new Instruction("br", INT),
            new Instruction("brt", INT),
            new Instruction("brf", INT),
            new Instruction("cconst", INT),
            new Instruction("iconst", INT),
            new Instruction("fconst", POOL),
            new Instruction("sconst", POOL),
            new Instruction("load", INT),
            new Instruction("gload", INT),
            new Instruction("fload", INT),
            new Instruction("store", INT),
            new Instruction("gstore", INT),
            new Instruction("fstore", INT),
            new Instruction("print"),
            new Instruction("struct", INT),
            new Instruction("null"),
            new Instruction("pop"),
            new Instruction("halt")
    };
}

