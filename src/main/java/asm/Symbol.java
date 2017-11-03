package asm;

/**
 * Created by sulvto on 17-11-4.
 */
public interface Symbol extends Literal {
    String name();

    String toString();

    String dump();
}
