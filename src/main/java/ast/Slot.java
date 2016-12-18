package ast;

/**
 * Created by sulvto on 16-12-18.
 */
public class Slot extends Node {

    private TypeNode typeNode;
    private String name;

    public Slot(TypeNode typeNode, String name) {
        this.typeNode = typeNode;
        this.name = name;
    }

    @Override
    public Location location() {
        return typeNode.location();
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
        dumper.printMember("typeNode", typeNode);
    }
}
