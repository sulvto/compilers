package ast;

import lexer.Token;
import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class LiteralNode extends ExprNode {
    protected TypeNode typeNode;

    public LiteralNode(Token token, TypeRef typeRef) {
        super(token);
        this.typeNode = new TypeNode(typeRef);
    }

}
