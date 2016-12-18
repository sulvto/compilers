package ast;

import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class TypeDefinition extends Node {
    protected Location location;
    protected String name;
    protected TypeNode typeNode;

    public TypeDefinition(Location location, TypeRef ref, String name) {
        this.location = location;
        this.name = name;
        this.typeNode = new TypeNode(ref);
    }

    @Override
    public Location location() {
        return location;
    }
}
