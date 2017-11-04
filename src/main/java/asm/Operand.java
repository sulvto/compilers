package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public abstract class Operand {
    abstract public String toSource(SymbolTable table);

    abstract public String dump();


    public boolean isRegister() {
        return false;
    }

    public boolean isMemoryReference() {
        return false;
    }
}
