package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public abstract class MemoryReference extends Operand implements Comparable<MemoryReference> {

    @Override
    public boolean isMemoryReference() {
        return true;
    }

    abstract public void fixOffset(long diff);

    abstract protected int cmp(DirectMemoryReference mem);

    abstract protected int cmp(IndirectMemoryReference mem);
}
