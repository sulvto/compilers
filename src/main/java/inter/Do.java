package inter;

import symbols.Type;

/**
 * Created by sulvto on 16-11-7.
 */
public class Do extends Stmt {
    Expr expr;
    Stmt stmt;

    public Do() {
        expr = null;
        stmt = null;
    }

    public void init(Expr x, Stmt s) {
        expr = x;
        stmt = s;
        if (expr.type != Type.Bool) expr.error("boolean required in while");
    }

    @Override
    public void gen(int b, int a) {
        after = a;
        int label = newlabel();
        stmt.gen(b, label);
        emitlabel(label);
        expr.jumping(b, 0);
    }
}
