package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public class ContinueNode extends StmtNode {


    public ContinueNode(Token token) {
        super(token);
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
