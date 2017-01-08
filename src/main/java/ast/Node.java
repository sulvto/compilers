package ast;

import java.io.PrintStream;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class Node implements Dumpable {
    abstract public Location location();

    protected abstract void doDump(Dumper dumper);

    public void dump() {
        dump(System.out);
    }

    public void dump(PrintStream stream) {
        dump(new Dumper(stream));
    }

    abstract public ASTNodeType nodeType();

    @Override
    public void dump(Dumper dumper) {
        dumper.printClass(this, location());
        doDump(dumper);
    }
}
