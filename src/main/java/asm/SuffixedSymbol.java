package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class SuffixedSymbol implements Symbol {
    private Symbol base;
    private String suffix;

    public SuffixedSymbol(Symbol base, String suffix) {
        this.base = base;
        this.suffix = suffix;
    }


    @Override
    public String name() {
        return base.name();
    }

    @Override
    public String toSource() {
        return base.toSource() + suffix;
    }

    @Override
    public String toSource(SymbolTable table) {
        return base.toSource(table) + suffix;
    }

    @Override
    public String toString() {
        return base.toString() + suffix;
    }

    @Override
    public String dump() {

        return "(SuffixedSymbol " + base.dump() + " " + TextUtils.dumpString(suffix) + ")";
    }

    @Override
    public boolean isZero() {
        return false;
    }

    @Override
    public int cmp(IntegerLiteral i) {
        return -1;
    }

    @Override
    public int cmp(NamedSymbol sym) {
        return -1;
    }

    @Override
    public int cmp(UnnamedSymbol sym) {
        return -1;
    }

    @Override
    public int cmp(SuffixedSymbol sym) {
        return toString().compareTo(sym.toString());
    }

    @Override
    public int compareTo(Literal o) {
        return -(o.cmp(this));
    }
}
