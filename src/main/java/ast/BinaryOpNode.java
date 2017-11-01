package ast;

import lexer.Token;

/**
 * x+y
 * x-y
 * Created by sulvto on 16-12-17.
 */
public class BinaryOpNode extends ExprNode {
    private ExprNode left, right;

    public BinaryOpNode(ExprNode left, Token token, ExprNode right) {
        super(token);
        this.left = left;
        this.right = right;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("operator", token.type);
        dumper.printMember("left", left);
        dumper.printMember("right", right);
    }
}
