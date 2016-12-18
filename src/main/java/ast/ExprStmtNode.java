package ast;

/**
 * Created by sulvto on 16-12-14.
 */
public class ExprStmtNode extends StmtNode {
    private ExprNode expr;

    public ExprStmtNode(Location location, ExprNode expr) {
        super(location);
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }
}
