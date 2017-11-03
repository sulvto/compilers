package asm;


/**
 * Created by sulvto on 17-11-4.
 */
public abstract class Assembly {
    abstract public String toSource(SymbolTable table);

    abstract public String dump();

    public boolean isInstruction() {
        return false;
    }

    public boolean isLabel() {
        return false;
    }

    public boolean isDirective() {
        return false;
    }

    public boolean isComment() {
        return false;
    }
}
