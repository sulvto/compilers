package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-17.
 */
public class LogicalAndNode extends BinaryOpNode {
    public LogicalAndNode(ExprNode left, Token token, ExprNode right) {
        super(left, token, right);
    }
}
