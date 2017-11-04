package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class NamedSymbol extends BaseSymbol {
    private String name;

    public NamedSymbol(String name) {
        this.name = name;
    }

    @Override
    public String name() {
        return name;
    }

    @Override
    public String toSource() {
        return name;
    }

    @Override
    public String toSource(SymbolTable table) {
        return name;
    }

    @Override
    public String toString() {
        return "#" + name;
    }

    @Override
    public String dump() {

        return "(NamedSymbol " + TextUtils.dumpSrting(name) + ")";
    }

    @Override
    public int cmp(IntegerLiteral i) {
        return 1;
    }

    @Override
    public int cmp(NamedSymbol sym) {
        return name.compareTo(sym.name);
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
