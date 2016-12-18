package ast;

/**
 * Created by sulvto on 16-12-17.
 */
public class VariableNode extends LHSNode {
    private String name;
    private Location location;

    public VariableNode(Location location, String name) {
        this.location = location;
        this.name = name;
    }

    @Override
    public Location location() {
        return location;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
    }
}
