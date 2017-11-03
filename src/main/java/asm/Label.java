package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class Label extends Assembly {

    private final Symbol symbol;

    public Label() {
        this(new UnnamedSymbol());
    }

    public Label(Symbol symbol) {
        this.symbol = symbol;
    }

    public Symbol symbol() {
        return symbol;
    }

    @Override
    public boolean isLabel() {
        return true;
    }

    @Override
    public String toSource(asm.SymbolTable table) {
        return symbol.toSource(table);
    }

    @Override
    public String dump() {
        return "(Label " + symbol.dump() + ")";
    }
}
