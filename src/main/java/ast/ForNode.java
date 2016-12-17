package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class ForNode extends StmtNode {
    private ExprNode init, incr, cond;
    private StmtNode body;

    public ForNode(ExprNode init, ExprNode cond, ExprNode incr, StmtNode body) {
        this.init = init;
        this.cond = cond;
        this.incr = incr;
        this.body = body;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
