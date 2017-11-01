package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class IfNode extends StmtNode {
    private ExprNode cond;
    private StmtNode thenBody;
    private StmtNode elseBody;

    public IfNode(Token token, ExprNode cond, StmtNode thenBody, StmtNode elseBody) {
        super(token);
        this.cond = cond;
        this.thenBody = thenBody;
        this.elseBody = elseBody;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("thenBody", thenBody);
        dumper.printMember("elseBody", elseBody);
    }
}
