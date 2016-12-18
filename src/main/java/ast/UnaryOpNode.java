package ast;

/**
 * -x
 * +x
 * !x
 * Created by sulvto on 16-12-8.
 */
public class UnaryOpNode extends ExprNode {
    private String op;
    private ExprNode expr;

    public UnaryOpNode(String op, ExprNode expr) {
        this.op = op;
        this.expr = expr;
    }

    @Override
    public Location location() {
        return expr.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("operator", op);
        dumper.printMember("expr", expr);
    }
}
