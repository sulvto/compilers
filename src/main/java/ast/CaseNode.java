package ast;

import lexer.Token;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class CaseNode extends StmtNode {
    private List<ExprNode> values;
    private BlockNode body;


    public CaseNode(Token token, List<ExprNode> values, BlockNode body) {
        super(token);
        this.values = values;
        this.body = body;
    }


    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("values", values);
        dumper.printMember("body", body);
    }


}
