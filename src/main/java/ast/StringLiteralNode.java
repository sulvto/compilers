package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class StringLiteralNode extends LiteralNode {
    private String value;

    public StringLiteralNode(String value) {
        this.value = value;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
