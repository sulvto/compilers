package inter;

import lexer.Token;

/**
 * Created by sulvto on 16-11-7.
 */
public class Or extends Logical {
    public Or(Token token, Expr x1, Expr x2) {
        super(token, x1, x2);
    }

    @Override
    public void jumping(int t, int f) {
        int label = t != 0 ? t : newlabel();
        expr1.jumping(label, 0);
        expr2.jumping(t, f);
        if(t==0) emitlabel(label);
    }
}
