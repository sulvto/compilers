package ir;

import asm.Type;

import java.util.List;

/**
 * Created by sulvto on 17-11-4.
 */
public class Call extends Expr {
    private Expr expr;
    private List<Expr> args;

    Call(Type type, Expr expr, List<Expr> args) {
        super(type);
        this.expr = expr;
        this.args = args;
    }

    public Expr expr() {
        return expr;
    }

    public List<Expr> args() {
        return args;
    }

    public int numArgs() {
        return args.size();
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("expr", expr);
        dumper.printMember("args", args);
    }
}
