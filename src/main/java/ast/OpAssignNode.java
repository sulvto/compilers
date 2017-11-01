package ast;

import lexer.Token;

/**
 * (a += 1)
 * Created by sulvto on 16-12-8.
 */
public class OpAssignNode extends AbstractAssignNode {

    public OpAssignNode(ExprNode left, Token token, ExprNode right) {
        super(left, token, right);
    }

    @Override
    protected void doDump(Dumper dumper) {

    }
}
