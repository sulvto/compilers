package ast;

import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public class IntegerLiteralNode extends LiteralNode {
    private long value;

    public IntegerLiteralNode(Location location, TypeRef typeRef, long value) {
        super(location, typeRef);
        this.value = value;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("typeNode", typeNode);
        dumper.printMember("value", value);
    }
}
