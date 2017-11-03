package ir;

import asm.Type;

/**
 * Created by sulvto on 17-11-3.
 */
public class Uni extends Expr {
    private Op op;
    private Expr expr;

    public Uni(Type type, Op op, Expr expr) {
        super(type);
        this.op = op;
        this.expr = expr;
    }

    public Expr expr() {
        return expr;
    }

    public Op op() {
        return op;
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("op", op);
        dumper.printMember("expr", expr);
    }
}
