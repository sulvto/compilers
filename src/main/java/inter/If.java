package inter;

import symbols.Type;

/**
 * if(Expr)Stmt
 * Created by sulvto on 16-11-7.
 */
public class If extends Stmt {
    Expr expr;
    Stmt stmt;

    public If(Expr e, Stmt s) {
        expr = e;
        stmt = s;
        if (expr.type != Type.Bool) expr.error("boolean required in if");
    }

    @Override
    public void gen(int b, int a) {
        int label = newlabel();
        expr.jumping(0, a);
        emitlabel(label);
        stmt.gen(label, a);
    }
}
