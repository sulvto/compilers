package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public class TypeNode extends Node {
    private String name;

    public TypeNode(String name) {
        this.name = name;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}
