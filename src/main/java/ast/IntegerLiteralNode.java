package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class IntegerLiteralNode extends LiteralNode {
    private long value;

    public IntegerLiteralNode(long value) {
        this.value = value;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
