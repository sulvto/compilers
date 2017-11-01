package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class DoWhileNode extends StmtNode {
    private ExprNode cond;
    private StmtNode body;

    public DoWhileNode(Token token, StmtNode body, ExprNode cond) {
        super(token);
        this.cond = cond;
        this.body = body;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("body", body);
    }
}
