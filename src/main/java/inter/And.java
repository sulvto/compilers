package inter;

import lexer.Token;

/**
 * Created by sulvto on 16-11-7.
 */
public class And extends Logical {
    public And(Token token, Expr x1, Expr x2) {
        super(token, x1, x2);
    }

    @Override
    public void jumping(int t, int f) {
        int label = f != 0 ? f : newlabel();
        expr1.jumping(0, label);
        expr2.jumping(t, f);
        if(f==0) emitlabel(label);
    }
}
