package ast;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class Node implements Dumpable {

    protected abstract void doDump(Dumper dumper);

    public void dump() {
        dump(new Dumper());
    }

    @Override
    public void dump(Dumper dumper) {
        // TODO
    }
}
