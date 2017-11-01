package ast;

import lexer.Token;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class ExprNode extends Node {
    public ExprNode(Token token) {
        super(token);
    }

    public abstract <S, E> E accept(ASTVisitor<S, E> visitor);
}
