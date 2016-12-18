package ast;

import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public class StringLiteralNode extends LiteralNode {
    private String value;

    public StringLiteralNode(Location location, TypeRef typeRef, String value) {
        super(location, typeRef);
        this.value = value;
    }


    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("value", value);
    }
}
