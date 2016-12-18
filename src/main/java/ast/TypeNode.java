package ast;

import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public class TypeNode extends Node {
    private String name;
    private TypeRef typeRef;

    public TypeNode(TypeRef typeRef) {
        this.typeRef = typeRef;
    }

    @Override
    public Location location() {
        return typeRef == null ? null : typeRef.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
