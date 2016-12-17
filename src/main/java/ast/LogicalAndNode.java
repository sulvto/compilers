package ast;

/**
 * Created by sulvto on 16-12-17.
 */
public class LogicalAndNode extends BinaryOpNode {
    public LogicalAndNode(ExprNode left, ExprNode right) {
        super(left, "&&", right);
    }
}
