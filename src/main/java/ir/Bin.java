package ir;

import asm.Type;

/**
 * Created by sulvto on 17-11-4.
 */
public class Bin extends Expr {
    private Op op;
    private Expr left, right;

    Bin(Type type, Op op, Expr left, Expr right) {
        super(type);
        this.op = op;
        this.left = left;
        this.right = right;
    }

    public Expr left() {
        return left;
    }

    public Expr right() {
        return right;
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
        dumper.printMember("op", op.toString());
        dumper.printMember("left", left);
        dumper.printMember("right", right);
    }
}
