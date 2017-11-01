package ast;

import lexer.Token;
import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class TypeDefinition extends Node {
    protected Location location;
    protected String name;
    protected TypeNode typeNode;

    public TypeDefinition(Token token, TypeRef ref, String name) {
        super(token);
        this.name = name;
        this.typeNode = new TypeNode(ref);
    }

}
