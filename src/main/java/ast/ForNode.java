package ast;

import entity.DefinedVariable;
import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class ForNode extends StmtNode {
    private DefinedVariable variable;

    private ExprNode init, cond, incr;
    private StmtNode body;

    public ForNode(Token token, ExprNode cond, ExprNode incr, StmtNode body) {
        super(token);
        this.cond = cond;
        this.incr = incr;
        this.body = body;
    }

    public ForNode(Token token, ExprNode init, ExprNode cond, ExprNode incr, StmtNode body) {
        super(token);
        this.init = init;
        this.cond = cond;
        this.incr = incr;
        this.body = body;
    }

    public ForNode(Token token, DefinedVariable variable, ExprNode cond, ExprNode incr, StmtNode body) {
        super(token);
        this.variable = variable;
        this.cond = cond;
        this.incr = incr;
        this.body = body;
    }

    @Override
    protected void doDump(Dumper dumper) {
        if (variable == null) {
            dumper.printMember("init", init);
        } else {
            dumper.printMember("init", variable);
        }
        dumper.printMember("cond", cond);
        dumper.printMember("incr", incr);
        dumper.printMember("body", body);
    }
}
