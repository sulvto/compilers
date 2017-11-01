package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-17.
 */
public abstract class LHSNode extends ExprNode {

    public LHSNode(Token token) {
        super(token);
    }
}
