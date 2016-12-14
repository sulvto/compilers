package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class DoWhileNode extends StmtNode {
    private ExprNode cond;
    private StmtNode body;

    public DoWhileNode(StmtNode body,ExprNode cond) {
        this.cond = cond;
        this.body = body;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
