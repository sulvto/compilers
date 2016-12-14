package ast;

/**
 * Created by sulvto on 16-12-14.
 */
public class ExprStmtNode extends StmtNode {
    private ExprNode expr;

    public ExprStmtNode(ExprNode expr) {
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
