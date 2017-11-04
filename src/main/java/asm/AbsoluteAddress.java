package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class AbsoluteAddress extends Operand {
    private Register register;

    public AbsoluteAddress(Register register) {
        this.register = register;
    }

    public Register register() {
        return register;
    }

    @Override
    public String toSource(SymbolTable table) {
        return "*" + register.toSource(table);
    }

    @Override
    public String dump() {
        return "(AbsoluteAddress " + register.dump() + ")";
    }
}
