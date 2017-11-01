package ast;

import lexer.Token;
import type.TypeRef;

/**
 * Created by sulvto on 16-12-8.
 */
public class StringLiteralNode extends LiteralNode {
    private String value;

    public StringLiteralNode(Token token, TypeRef typeRef, String value) {
        super(token, typeRef);
        this.value = value;
    }


    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("value", value);
    }
}
