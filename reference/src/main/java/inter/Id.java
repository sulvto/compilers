package inter;

import symbols.Type;

/**
 * Created by sulvto on 16-11-6.
 */
public class Id extends Expr {
    public int offset;

    public Id(Word id, Type p, int b) {
        super(id, p);
        offset = b;
    }
}
