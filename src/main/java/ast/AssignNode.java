package ast;

import lexer.Token;

/**
 * (a = 1)
 * Created by sulvto on 16-12-8.
 */
public class AssignNode extends AbstractAssignNode {

    public AssignNode(ExprNode left, Token token, ExprNode right) {
        super(left, token, right);
    }
}
