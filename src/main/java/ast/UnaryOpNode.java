package ast;

import lexer.Token;

/**
 * -x
 * +x
 * !x
 * Created by sulvto on 16-12-8.
 */
public class UnaryOpNode extends ExprNode {
    private ExprNode expr;

    public UnaryOpNode(Token token, ExprNode expr) {
        super(token);
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("operator", token.type);
        dumper.printMember("expr", expr);
    }

    public ExprNode expr() {
        return expr;
    }
}
