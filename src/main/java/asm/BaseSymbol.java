package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public abstract class BaseSymbol implements Symbol {
    @Override
    public boolean isZero() {
        return false;
    }
}
