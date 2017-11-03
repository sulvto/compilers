package ir;

/**
 * Created by sulvto on 17-11-3.
 */
public abstract class Stmt implements Dumpable {
    protected String location;

    public Stmt(String location) {
        this.location = location;
    }

    public String location() {
        return location;
    }

    @Override
    public void dump(Dumper dumper) {
        dumper.printClass(this, location);
        doDump(dumper);
    }

    protected abstract void doDump(Dumper dumper);

    abstract public <S, E> S accept(IRVisitor<S, E> visitor);

}
