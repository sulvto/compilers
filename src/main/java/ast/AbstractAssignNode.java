package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public abstract class AbstractAssignNode extends ExprNode {
    protected ExprNode lhs, rhs;

    public AbstractAssignNode(ExprNode left, Token token, ExprNode right) {
        super(token);
        this.lhs = left;
        this.rhs = right;
    }

    public ExprNode lhs() {
        return lhs;
    }

    public ExprNode rhs() {
        return rhs;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("lhs", lhs);
        dumper.printMember("rhs", rhs);
    }
}
