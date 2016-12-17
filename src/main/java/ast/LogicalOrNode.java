package ast;

/**
 * Created by sulvto on 16-12-17.
 */
public class LogicalOrNode extends BinaryOpNode {

    public LogicalOrNode(ExprNode left, ExprNode right) {
        super(left, "||", right);
    }
}
