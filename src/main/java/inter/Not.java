package inter;

import lexer.Token;

/**
 * Created by sulvto on 16-11-7.
 */
public class Not extends Logical {
    public Not(Token token, Expr x1, Expr x2) {
        super(token, x1, x2);
    }

    @Override
    public void jumping(int t, int f) {
        expr2.jumping(f, t);
    }

    @Override
    public String toString() {
        return op.toString() + " " + expr2.toString();
    }
}
