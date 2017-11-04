package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class UnnamedSymbol extends BaseSymbol {
    @Override
    public String name() {
        throw new Error("unnamed symbol");
    }

    @Override
    public String toSource() {
        throw new Error("UnnamedSymbol#toSource() called");
    }

    @Override
    public String toSource(SymbolTable table) {
        return table.symbolString(this);
    }

    @Override
    public String toString() {
        return super.toString();
    }

    @Override
    public String dump() {

        return "(UnnamedSymbol @" + Integer.toHexString(hashCode()) + ")";
    }

    @Override
    public int cmp(IntegerLiteral i) {
        return 1;
    }

    @Override
    public int cmp(NamedSymbol sym) {
        return 1;
    }

    @Override
    public int cmp(UnnamedSymbol sym) {
        return toString().compareTo(sym.toString());
    }

    @Override
    public int cmp(SuffixedSymbol sym) {
        return 1;
    }

    @Override
    public int compareTo(Literal o) {
        return -(o.cmp(this));
    }
}
