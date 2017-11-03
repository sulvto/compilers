package ast;

import type.Type;
import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public class TypeNode extends Node {
    private String name;
    private TypeRef typeRef;
    private Type type;
    private boolean resolved;

    public TypeNode(TypeRef typeRef) {
        super(typeRef.token);
        this.typeRef = typeRef;
    }

    public TypeRef typeRef() {
        return typeRef;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("typeRef", typeRef);
        dumper.printMember("type", type);
    }


    public Type type() {
        if (type == null) {
            throw new Error("TypeNode not resolved:" + typeRef);
        }
        return type;
    }

    public void setType(Type type) {
        if (this.type != null) {
            throw new Error("TypeNode#setType called twice");
        }
        this.type = type;
    }

    public boolean isResolved() {
        return resolved;
    }
}
