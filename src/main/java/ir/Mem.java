package ir;

import asm.Type;

/**
 * Created by sulvto on 17-11-4.
 */
public class Mem extends Expr {
    private Expr expr;

    public Mem(Type type, Expr expr) {
        super(type);
        this.expr = expr;
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }
}
