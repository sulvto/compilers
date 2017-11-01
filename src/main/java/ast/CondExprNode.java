package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class CondExprNode extends ExprNode {
    private ExprNode cond, thenExpr, elseExpr;

    public CondExprNode(ExprNode cond, ExprNode thenExpr, ExprNode elseExpr) {
        super(cond.token);
        this.cond = cond;
        this.thenExpr = thenExpr;
        this.elseExpr = elseExpr;
    }

    public ExprNode cond() {
        return cond;
    }

    public ExprNode elseExpr() {
        return elseExpr;
    }

    public ExprNode thenExpr() {
        return thenExpr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("thenExpr", thenExpr);
        dumper.printMember("elseExpr", elseExpr);
    }
}
