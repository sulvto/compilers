package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class StmtNode extends Node {
    protected Location location;

    public StmtNode(Location location) {
        this.location = location;
    }

    @Override
    public Location location() {
        return location;
    }
}
