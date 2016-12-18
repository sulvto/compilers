package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class CondExprNode extends ExprNode {
    private ExprNode cond, thenExpr, elseExpr;

    public CondExprNode(ExprNode cond, ExprNode thenExpr, ExprNode elseExpr) {
        this.cond = cond;
        this.thenExpr = thenExpr;
        this.elseExpr = elseExpr;
    }

    @Override
    public Location location() {
        return cond.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("thenExpr", thenExpr);
        dumper.printMember("elseExpr", elseExpr);
    }
}
