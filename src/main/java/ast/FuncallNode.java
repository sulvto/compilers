package ast;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class FuncallNode extends ExprNode {

    private ExprNode expr;
    private List<ExprNode> args;

    public FuncallNode(ExprNode expr, List<ExprNode> args) {
        this.expr = expr;
        this.args = args;
    }

    @Override
    public Location location() {
        return expr.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
        dumper.printMember("args", args);
    }
}
