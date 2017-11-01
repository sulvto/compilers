package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class WhileNode extends StmtNode {
    private ExprNode cond;
    private StmtNode body;

    public WhileNode(Token token, ExprNode cond, StmtNode body) {
        super(token);
        this.cond = cond;
        this.body = body;
    }

    public ExprNode cond() {
        return cond;
    }

    public StmtNode body() {
        return body;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("cond", cond);
        dumper.printMember("body", body);
    }

}
