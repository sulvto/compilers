package inter;

import lexer.Token;
import symbols.Type;

/**
 * 单目运算符
 * Created by sulvto on 16-11-6.
 */
public class Unary extends Op {
    public Expr expr;

    public Unary(Token token, Expr x) {
        super(token, null);
        expr = x;
        type = Type.max(Type.Int, expr.type);
        if (type == null) {
            error("type error");
        }
    }

    @Override
    public Expr gen() {
        return new Unary(op, expr.reduce());
    }

    @Override
    public String toString() {
        return op.toString() + " " + expr.toString();
    }
}
