package symbols;

import lexer.Tag;

/**
 * Created by sulvto on 16-11-6.
 */
public class Array extends Type {
    public Type of;
    public int size = 1;

    public Array(int sz, Type p) {
        super("[]", Tag.INDEX, sz * p.width);
        size = sz;
        of = p;
    }

    @Override
    public String toString() {
        return "[" + size + "]" + of.toString();
    }
}
