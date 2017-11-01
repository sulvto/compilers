package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class StmtNode extends Node {

    public StmtNode(Token token) {
        super(token);
    }

}
