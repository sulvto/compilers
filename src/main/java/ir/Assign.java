package ir;

/**
 * Created by sulvto on 17-11-3.
 */
public class Assign extends Stmt {
    private Expr lhs, rhs;

    public Assign(String location, Expr lhs, Expr rhs) {
        super(location);
        this.lhs = lhs;
        this.rhs = rhs;
    }

    public Expr lhs() {
        return lhs;
    }

    public Expr rhs() {
        return rhs;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("lhs", lhs);
        dumper.printMember("rhs", rhs);
    }

    @Override
    public <S, E> S accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }
}
