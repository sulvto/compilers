package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public abstract class Register extends Operand {
    @Override
    public boolean isRegister() {
        return true;
    }

    abstract public String toSource(SymbolTable table);

    abstract public String dump();
}
