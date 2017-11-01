package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class ReturnNode extends StmtNode {
    private ExprNode expr;

    public ReturnNode(Token token, ExprNode expr) {
        super(token);
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }

    public ExprNode getExpr() {
        return expr;
    }

}
