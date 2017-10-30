package assembler;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by sulvto on 17-10-29.
 */
public class BytecodeAssembler extends AssemblerParser {
    public static final int INITIAL_CODE_SIZE = 1024;
    protected Map<String, Integer> instructionOpcodeMapping = new HashMap<>();
    protected Map<String, LabelSymbol> labels = new HashMap<>();

    protected List<Object> constPool = new ArrayList<>();
    protected int ip = 0;
    protected byte[] code = new byte[INITIAL_CODE_SIZE];
    protected int dataSize;
    protected FunctionSymbol mainFunction;

    public BytecodeAssembler(InputStream input, BytecodeDefinition.Instruction[] instructions) {
        super(input);
        for (int i = 1; i < instructions.length; i++) {
            instructionOpcodeMapping.put(instructions[i].name.toLowerCase(), i);
        }
    }

    @Override
    protected void gen(Token instrToken) {
        String instrName = instrToken.text;
        Integer opcodeI = instructionOpcodeMapping.get(instrName);
        if (opcodeI == null) {
            System.err.println("line " + instrToken.line + ": Unknown instruction:" + instrName);
            return;
        }
        int opcode = opcodeI.intValue();
        ensureCapacity(ip + 1);
        code[ip++] = (byte) (opcode & 0xFF);
    }

    @Override
    protected void gen(Token instrToken, Token oToken1) {
        gen(instrToken);
        genOperand(oToken1);
    }

    @Override
    protected void gen(Token instrToken, Token oToken1, Token oToken2) {
        gen(instrToken, oToken1);
        genOperand(oToken2);
    }

    @Override
    protected void gen(Token instrToken, Token oToken1, Token oToken2, Token oToken3) {
        gen(instrToken, oToken1, oToken2);
        genOperand(oToken2);
    }

    @Override
    protected void defineDataSize(int dataSize) {
        this.dataSize = dataSize;
    }

    @Override
    protected void defineFunction(Token idToken, int args, int locals) {
        String name = idToken.text;
        FunctionSymbol fun = new FunctionSymbol(name, args, locals, ip);
        if (name.equals("main")) mainFunction = fun;
        if (constPool.contains(fun)) constPool.set(constPool.indexOf(fun), fun);
        else getConstantPoolIndex(fun);
    }

    private int getConstantPoolIndex(Object o) {
        if (constPool.contains(o)) return constPool.indexOf(o);
        constPool.add(o);
        return constPool.size() - 1;
    }

    private int getFunctionIndex(String id) {
        int i = constPool.indexOf(new FunctionSymbol(id));
        if (i >= 0) return i;
        return getConstantPoolIndex(new FunctionSymbol(id));
    }

    @Override
    protected void defineLabel(Token idToken) {
        String id = idToken.text;
        LabelSymbol label = labels.get(id);
        if (label == null) {
            labels.put(id, new LabelSymbol(id, ip, false));
        } else {
            if (label.isForwardRef) {
                label.isDefined = true;
                label.address = ip;
                label.resolveForwardReferences(code);
            } else {
                // redefinition of symbol
                System.err.println("line " + idToken.line +
                        ": redefinition of symbol " + id);
            }
        }
    }

    private int getLabelAddress(String id) {
        LabelSymbol label = labels.get(id);
        if (label == null) {
            label = new LabelSymbol(id, ip, true);
            label.isDefined = false;
            labels.put(id, label);
        } else {
            if (label.isForwardRef) {
                label.addForwardReference(ip);
            } else {
                return label.address;
            }
        }
        return 0;
    }

    private void genOperand(Token operandToken) {
        String text = operandToken.text;
        int v = 0;
        switch (operandToken.type) {
            case Type.INT:
                v = Integer.parseInt(text);
                break;
            case Type.CHAR:
                v = Character.valueOf(text.charAt(0));
                break;
            case Type.STRING:
                v = getConstantPoolIndex(text);
                break;
            case Type.FLOAT:
                v = getConstantPoolIndex(Float.valueOf(text));
                break;
            case Type.ID:
                v = getLabelAddress(text);
                break;
            case Type.FUNC:
                v = getFunctionIndex(text);
                break;
            case Type.REG:
                v = getRegisterNumber(operandToken);
                break;
        }

        ensureCapacity(ip + 4);
        writeInt(code, ip, v);
        ip += 4;
    }


    private int getRegisterNumber(Token regToken) {
        return Integer.valueOf(regToken.text.substring(1));
    }

    @Override
    protected void checkForUnresolvedReferences() {
        labels.values().forEach(label -> {
            if (!label.isDefined) {
                System.err.println("unresolved reference:" + label.name);
            }
        });
    }

    public FunctionSymbol getMainFunction() {
        return mainFunction;
    }

    public Object[] getConstantPool() {
        return constPool.toArray();
    }

    public int getCodeMemorySize() {
        return ip;
    }

    public byte[] getMachineCode() {
        return code;
    }

    public int getDataSize() {
        return dataSize;
    }

    protected void ensureCapacity(int index) {
        if (index >= code.length) {
            int newSize = Math.max(index, code.length) * 1;
            byte[] bigger = new byte[newSize];
            System.arraycopy(code, 0, bigger, 0, code.length);
            code = bigger;
        }
    }

    public static void writeInt(byte[] bytes, int index, int value) {
        bytes[index + 0] = (byte) ((value >> (8 * 3)) & 0xFF);
        bytes[index + 1] = (byte) ((value >> (8 * 2)) & 0xFF);
        bytes[index + 2] = (byte) ((value >> (8 * 1)) & 0xFF);
        bytes[index + 3] = (byte) (value & 0xFF);
    }

    public static int getInt(byte[] memory, int index) {
        int b1 = memory[index++] & 0xFF;
        int b2 = memory[index++] & 0xFF;
        int b3 = memory[index++] & 0xFF;
        int b4 = memory[index++] & 0xFF;
        return b1 << (8 * 3) | b2 << (8 * 2) | b3 << (8) | b4;
    }
}
