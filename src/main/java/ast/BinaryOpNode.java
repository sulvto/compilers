package ast;

/**
 * x+y
 * x-y
 * Created by sulvto on 16-12-17.
 */
public class BinaryOpNode extends ExprNode {
    private String operator;
    private ExprNode left, right;

    public BinaryOpNode(ExprNode left, String op, ExprNode right) {
        this.operator = op;
        this.left = left;
        this.right = right;
    }

    @Override
    public Location location() {
        return left.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("operator", operator);
        dumper.printMember("left", left);
        dumper.printMember("right", right);
    }
}
