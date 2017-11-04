package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public class DirectMemoryReference extends MemoryReference {
    private Literal value;

    public DirectMemoryReference(Literal value) {
        this.value = value;
    }

    public Literal value() {
        return value;
    }

    @Override
    public void fixOffset(long diff) {
        throw new Error("DirectMemoryReference#fixOffset");
    }

    @Override
    protected int cmp(DirectMemoryReference mem) {
        return value.compareTo(mem.value);
    }

    @Override
    protected int cmp(IndirectMemoryReference mem) {
        return -1;
    }

    @Override
    public String toString() {
        return toSource(SymbolTable.dummy());
    }

    @Override
    public String toSource(SymbolTable table) {
        return value.toSource(table);
    }

    @Override
    public String dump() {
        return "(DirectMemoryReference " + value.dump() + ")";
    }

    @Override
    public int compareTo(MemoryReference mem) {
        return -(mem.cmp(this));
    }
}
