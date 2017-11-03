package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public interface Literal extends Comparable<Literal> {
    String toSource();

    String toSource(SymbolTable table);

    String dump();

    boolean isZero();
}
