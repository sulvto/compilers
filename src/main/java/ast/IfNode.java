package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class IfNode extends StmtNode {
    private ExprNode cond;
    private StmtNode thenBody;
    private StmtNode elseBody;

    public IfNode(ExprNode cond, StmtNode thenBody, StmtNode elseBody) {
        this.cond = cond;
        this.thenBody = thenBody;
        this.elseBody = elseBody;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
