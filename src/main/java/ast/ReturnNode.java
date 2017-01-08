package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class ReturnNode extends StmtNode {
    private ExprNode expr;

    public ReturnNode(Location location, ExprNode expr) {
        super(location);
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }

    public ExprNode getExpr() {
        return expr;
    }

    @Override
    public ASTNodeType nodeType() {
        return ASTNodeType.RETURN;
    }
}
