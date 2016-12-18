package ast;

import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class LiteralNode extends ExprNode {
    protected Location location;
    protected TypeNode typeNode;

    public LiteralNode(Location location, TypeRef typeRef) {
        this.location = location;
        this.typeNode = new TypeNode(typeRef);
    }

    @Override
    public Location location() {
        return location;
    }
}
