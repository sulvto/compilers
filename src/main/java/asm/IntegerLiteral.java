package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class IntegerLiteral implements Literal {
    private long value;

    public IntegerLiteral(long value) {
        this.value = value;
    }

    public long value() {
        return value;
    }

    @Override
    public String toSource() {
        return new Long(value).toString();
    }

    @Override
    public String toSource(SymbolTable table) {
        return toSource();
    }

    @Override
    public String dump() {
        return null;
    }

    @Override
    public boolean isZero() {
        return value == 0;
    }

    @Override
    public int cmp(IntegerLiteral i) {
        return new Long(value).compareTo(new Long(i.value));
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
        return -1;
    }

    @Override
    public String toString() {
        return new Long(value).toString();
    }

    @Override
    public int compareTo(Literal o) {
        return -(o.cmp(this));
    }
}
