package ast;

/**
 * Created by sulvto on 16-12-18.
 */
public class AbstractAssignNode extends ExprNode {
    protected ExprNode lhs, rhs;

    public AbstractAssignNode(ExprNode left, ExprNode right) {
        this.lhs = left;
        this.rhs = right;
    }

    @Override
    public Location location() {
        return lhs.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("lhs", lhs);
        dumper.printMember("rhs", rhs);
    }
}
