package ir;

import asm.Label;

/**
 * Created by sulvto on 17-11-3.
 */
public class Cjump extends Stmt {
    private Expr cond;
    private Label thenLabel;
    private Label elseLabel;

    public Cjump(String location, Expr cond, Label thenLabel, Label elseLabel) {
        super(location);
        this.cond = cond;
        this.thenLabel = thenLabel;
        this.elseLabel = elseLabel;
    }

    public Expr cond() {
        return cond;
    }

    public Label elseLabel() {
        return elseLabel;
    }

    public Label thenLabel() {
        return thenLabel;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("thenLabel", thenLabel);
        dumper.printMember("elseLabel", elseLabel);
    }

    @Override
    public <S, E> S accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }
}
