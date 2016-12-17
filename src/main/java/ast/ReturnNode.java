package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class ReturnNode extends StmtNode {
    private ExprNode expr;

    public ReturnNode(ExprNode expr) {
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
