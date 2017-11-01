package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-14.
 */
public class ExprStmtNode extends StmtNode {
    private ExprNode expr;

    public ExprStmtNode(Token token, ExprNode expr) {
        super(token);
        this.expr = expr;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("expr", expr);
    }
}
