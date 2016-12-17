package ast;

/**
 * x+y
 * x-y
 * Created by sulvto on 16-12-17.
 */
public class BinaryOpNode extends ExprNode {
    private String operator;
    private ExprNode left, right;

    public BinaryOpNode( ExprNode left,String op, ExprNode right) {
        this.operator = op;
        this.left = left;
        this.right = right;
    }

    @Override
    protected void doDump(Dumper dumper) {

    }
}
