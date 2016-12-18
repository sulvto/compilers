package ast;

/**
 * (a = 1)
 * Created by sulvto on 16-12-8.
 */
public class AssignNode extends AbstractAssignNode {

    public AssignNode(ExprNode left, ExprNode right) {
        super(left, right);
    }
}
