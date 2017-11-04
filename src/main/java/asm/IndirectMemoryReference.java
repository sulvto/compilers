package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class IndirectMemoryReference extends MemoryReference {
    private Literal offset;
    private Register base;
    private boolean fixed;

    public IndirectMemoryReference(long offset, Register base) {
        this(new IntegerLiteral(offset), base, true);
    }

    public IndirectMemoryReference(Symbol offset, Register base) {
        this(offset, base, true);
    }

    static IndirectMemoryReference relocatable(long offset, Register base) {
        return new IndirectMemoryReference(new IntegerLiteral(offset), base, false);
    }

    public IndirectMemoryReference(Literal offset, Register base, boolean fixed) {
        this.offset = offset;
        this.base = base;
        this.fixed = fixed;
    }


    public Literal offset() {
        return offset;
    }

    @Override
    public void fixOffset(long diff) {
        if (fixed) {
            throw new Error("must not happen: fixed == true");
        }

        long curr = ((IntegerLiteral) offset).value();
        this.offset = new IntegerLiteral(curr + diff);
        this.fixed = true;
    }

    public Register base() {
        return base;
    }

    @Override
    protected int cmp(DirectMemoryReference mem) {
        return -1;
    }

    @Override
    protected int cmp(IndirectMemoryReference mem) {
        return offset.compareTo(mem.offset);
    }

    @Override
    public String toString() {
        return toSource(SymbolTable.dummy());
    }

    @Override
    public String toSource(SymbolTable table) {
        if (!fixed) {
            throw new Error("must not happen:writing unfixed variable");
        }
        return (offset.isZero() ? "" : offset.toSource(table)) + "(" + base.toSource(table) + ")";
    }

    @Override
    public String dump() {
        return "(IndirectMemoryReference " + (fixed ? "" : "*") + offset.dump() + " " + base.dump() + ")";
    }

    @Override
    public int compareTo(MemoryReference mem) {
        return -(mem.cmp(this));
    }
}
