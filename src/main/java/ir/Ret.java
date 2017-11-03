package ir;

/**
 * Created by sulvto on 17-11-3.
 */
public class Ret extends Stmt {
    private Expr expr;

    public Ret(String location, Expr expr) {
        super(location);
        this.expr = expr;
    }

    public Expr expr() {
        return expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }

    @Override
    public <S, E> S accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }
}
